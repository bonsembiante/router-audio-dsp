#include <pthread.h>
#include "sound.h"
#include "ring_buffer.h"

snd_pcm_t *playback_handle;
snd_pcm_t *capture_handle;

void setscheduler(void)
{
  struct sched_param sched_param;

  if (sched_getparam(0, &sched_param) < 0)
  {
    printf("Scheduler getparam failed...\n");
    return;
  }
  sched_param.sched_priority = sched_get_priority_max(SCHED_RR);
  if (!sched_setscheduler(0, SCHED_RR, &sched_param))
  {
    printf("Scheduler set to Round Robin with priority %i...\n", sched_param.sched_priority);
    fflush(stdout);
    return;
  }
  printf("Scheduler set to Round Robin with priority %i FAILED\n", sched_param.sched_priority);
}

/**
 * Main
 */
int main(int argc, char *argv[])
{
  // Get parameters from command arguments
  int opt;
  stream_params params = {};

  params.delay_time = -1.0;
  params.dry_wet = -1.0;
  params.feedback = -1.0;

  char *capture_device = NULL;
  char *playback_device = NULL;
  while ((opt = getopt(argc, argv, ":p:c:t:f:w:")) != -1)
  {
    switch (opt)
    {
    case 'p':
      playback_device = optarg;
      break;
    case 'c':
      capture_device = optarg;
      break;
    case 't':
      params.delay_time = (float)strtod(optarg, NULL);
      break;
    case 'f':
      params.feedback = (float)strtod(optarg, NULL);
      break;
    case 'w':
      params.dry_wet = (float)strtod(optarg, NULL);
      break;
    case ':':
      printf("option needs a value\n");
      break;
    case '?':
      printf("unknown option: %c\n", optopt);
      break;
    }
  }

  if (params.delay_time == -1 ||
      params.feedback == -1 ||
      params.dry_wet == -1 ||
      capture_device == NULL ||
      playback_device == NULL)
  {
    printf("Usage: delay_processor -p <playback_device> -c <capture_device> -t <delay_time in seconds> -f <delay_feedback 0 - 1> -w <delay_dry_wet 0 - 1>\n");
    return 0;
  }

  // Set max priority to the process in order to reduce latency
  setscheduler();

  int err;
  int numBytes;

  // Init capture device
  if ((err = snd_pcm_open(&capture_handle, capture_device, SND_PCM_STREAM_CAPTURE, 0)) < 0)
  {
    fprintf(stderr, "cannot open audio device '%s'. Error: %s\n", capture_device, snd_strerror(err));
    exit(1);
  }
  setup_alsa_handle(capture_handle, SND_PCM_STREAM_CAPTURE);

  if ((err = snd_pcm_start(capture_handle)) < 0)
  {
    fprintf(stderr, "cannot prepare audio interface for use(%s)\n",
            snd_strerror(err));
    return err;
  }

  // Init playback device
  if ((err = snd_pcm_open(&playback_handle, playback_device, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
  {
    fprintf(stderr, "cannot open audio device '%s'. Error: %s\n", playback_device, snd_strerror(err));
    exit(1);
  }
  setup_alsa_handle(playback_handle, SND_PCM_STREAM_PLAYBACK);

  if ((err = snd_pcm_prepare(playback_handle)) < 0)
  {
    fprintf(stderr, "cannot prepare audio interface for use(%s)\n",
            snd_strerror(err));
    return err;
  }

  // Dump pcm settings info
  snd_output_t *output = NULL;
  snd_output_stdio_attach(&output, stderr, 0);
  snd_pcm_dump(capture_handle, output);
  snd_pcm_dump(playback_handle, output);

  // Start stream
  start_stream(playback_handle, capture_handle, &params);

  return 0;
}
