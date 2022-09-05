#include "ring_buffer.h"
#include "sound.h"
#include "processor.h"

/**
 * Setup an alsa handle
 */
int setup_alsa_handle(snd_pcm_t *handle, snd_pcm_stream_t stream)
{

  int err = 0;
  snd_pcm_hw_params_t *hw_params;
  snd_pcm_sw_params_t *sw_params;

  if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0)
  {
    fprintf(stderr, "cannot allocate hardware parameter structure (%s)\n",
            snd_strerror(err));
    exit(1);
  }

  if ((err = snd_pcm_hw_params_any(handle, hw_params)) < 0)
  {
    fprintf(stderr, "cannot initialize hardware parameter structure (%s)\n",
            snd_strerror(err));
    exit(1);
  }

  if ((err = snd_pcm_hw_params_set_access(handle, hw_params,
                                          SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
  {
    fprintf(stderr, "cannot set access type (%s)\n", snd_strerror(err));
    exit(1);
  }

  if ((err = snd_pcm_hw_params_set_format(handle, hw_params, AUDIO_FORMAT)) < 0)
  {
    fprintf(stderr, "cannot set sample format (%s)\n", snd_strerror(err));
    exit(1);
  }

  unsigned int rate = SAMPLE_RATE;
  if ((err = snd_pcm_hw_params_set_rate_near(handle, hw_params, &rate, 0)) <
      0)
  {
    fprintf(stderr, "cannot set sample rate (%s)\n", snd_strerror(err));
    exit(1);
  }

  snd_pcm_uframes_t buffer_size = FRAMES_PER_BUFFER;
  if ((err = snd_pcm_hw_params_set_buffer_size_near(handle, hw_params, &buffer_size)) <
      0)
  {
    fprintf(stderr, "cannot set buffer size (%s)\n", snd_strerror(err));
    exit(1);
  }

  int dir;
  snd_pcm_uframes_t period_size = PERIOD_SIZE_IN_FRAMES;
  if ((err = snd_pcm_hw_params_set_period_size_near(handle, hw_params, &period_size, &dir)) <
      0)
  {
    fprintf(stderr, "cannot set period size (%s)\n", snd_strerror(err));
    exit(1);
  }

  int channels;
  if (stream == SND_PCM_STREAM_CAPTURE)
  {
    channels = CAPTURE_CHANNELS;
  }
  else
  {
    channels = PLAYBACK_CHANNELS;
  }
  if ((err = snd_pcm_hw_params_set_channels(handle, hw_params, channels)) < 0)
  {
    fprintf(stderr, "cannot set channel count (%s)\n", snd_strerror(err));
    exit(1);
  }

  if ((err = snd_pcm_hw_params(handle, hw_params)) < 0)
  {
    fprintf(stderr, "cannot set parameters (%s)\n", snd_strerror(err));
    exit(1);
  }

  snd_pcm_hw_params_free(hw_params);

  if ((err = snd_pcm_sw_params_malloc(&sw_params)) < 0)
  {
    fprintf(stderr, "cannot allocate software parameters structure(%s)\n",
            snd_strerror(err));
    return err;
  }
  if ((err = snd_pcm_sw_params_current(handle, sw_params)) < 0)
  {
    fprintf(stderr, "cannot initialize software parameters structure(%s)\n",
            snd_strerror(err));
    return err;
  }
  if ((err = snd_pcm_sw_params_set_avail_min(handle, sw_params, AVAILABLE_MIN)) < 0)
  {
    fprintf(stderr, "cannot set minimum available count(%s)\n",
            snd_strerror(err));
    return err;
  }
  if ((err = snd_pcm_sw_params_set_start_threshold(handle, sw_params, 0U)) < 0)
  {
    fprintf(stderr, "cannot set start mode(%s)\n",
            snd_strerror(err));
    return err;
  }
  if ((err = snd_pcm_sw_params(handle, sw_params)) < 0)
  {
    fprintf(stderr, "cannot set software parameters(%s)\n",
            snd_strerror(err));
    return err;
  }
  return 0;
}

/**
 * Start stream
 */
void start_stream(snd_pcm_t *_playback_handle,
                  snd_pcm_t *_capture_handle,
                  stream_params *params)
{
  // Setup buffers
  audio_sample *in_buffer;
  audio_sample *out_buffer;
  audio_sample *processed_out_buffer;

  int in_buffer_samples = FRAMES_PER_BUFFER * CAPTURE_CHANNELS;
  in_buffer = (audio_sample *)malloc(in_buffer_samples * sizeof(audio_sample));

  int out_buffer_samples = FRAMES_PER_BUFFER * PLAYBACK_CHANNELS;
  out_buffer = (audio_sample *)malloc(out_buffer_samples * sizeof(audio_sample));
  processed_out_buffer = (audio_sample *)malloc(out_buffer_samples * sizeof(audio_sample));

  ring_buffer_t ring_buffer = {};
  ring_buffer_init(&ring_buffer, RING_BUFFER_SIZE, "stream");

  // Setup delay processor
  setup_processor(SAMPLE_RATE, params->delay_time, params->dry_wet, params->feedback);

  // Start stream loop
  int err;
  int avail;
  while (1)
  {
    if ((err = snd_pcm_wait(_capture_handle, 1000)) < 0)
    {
      fprintf(stderr, "poll failed(%s)\n", strerror(errno));
      break;
    }

    avail = snd_pcm_avail_update(_capture_handle);
    if (avail > 0)
    {
      if (avail > FRAMES_PER_BUFFER)
        avail = FRAMES_PER_BUFFER;

      snd_pcm_readi(_capture_handle, in_buffer, avail);

      // Save samples in our ring buffer
      ring_buffer_put(&ring_buffer, in_buffer, avail);
    }

    if ((err = snd_pcm_wait(_playback_handle, 1000)) < 0)
    {
      fprintf(stderr, "poll failed(%s)\n", strerror(errno));
      break;
    }

    avail = snd_pcm_avail_update(_playback_handle);
    if (avail > 0)
    {
      if (avail > FRAMES_PER_BUFFER)
        avail = FRAMES_PER_BUFFER;

      // Get samples from our ring buffer
      ring_buffer_get(&ring_buffer, out_buffer, avail);

      // Process signal before it is written
      process_buffer(out_buffer, processed_out_buffer, avail);

      // Write it
      snd_pcm_writei(_playback_handle, processed_out_buffer, avail);
    }
  }

  ring_buffer_destroy(&ring_buffer);
}
