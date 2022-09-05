#ifndef SOUND_H
#define SOUND_H

#include <alsa/asoundlib.h>

#ifndef CAPTURE_CHANNELS
  #define CAPTURE_CHANNELS 1
#endif

#ifndef PLAYBACK_CHANNELS
  #define PLAYBACK_CHANNELS 1
#endif

#ifndef SAMPLE_RATE
  #define SAMPLE_RATE 44100
#endif

#ifndef FRAMES_PER_BUFFER
  #define FRAMES_PER_BUFFER 2046
#endif

#ifndef RING_BUFFER_SIZE
  #define RING_BUFFER_SIZE FRAMES_PER_BUFFER * 16
#endif

#ifndef AVAILABLE_MIN
  #define AVAILABLE_MIN 1024
#endif

#ifndef PERIOD_SIZE_IN_FRAMES
  #define PERIOD_SIZE_IN_FRAMES 1
#endif

#ifndef AUDIO_FORMAT
  // Received from -D PCM_FORMAT_BE argument when gcc is called
  #ifdef PCM_FORMAT_BE
    #define AUDIO_FORMAT SND_PCM_FORMAT_S16_BE
  #else
    #define AUDIO_FORMAT SND_PCM_FORMAT_S16_LE
  #endif
#endif

typedef struct
{
  float delay_time;
  float dry_wet;
  float feedback;
} stream_params;

typedef short audio_sample;
#define SAMPLE_SILENCE 0

int setup_alsa_handle(snd_pcm_t *handle, snd_pcm_stream_t stream);

void start_stream(snd_pcm_t *_playback_handle,
                  snd_pcm_t *_capture_handle,
                  stream_params *params);

#endif