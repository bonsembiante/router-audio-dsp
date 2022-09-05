#include "processor.h"
#include "ring_buffer.h"

ring_buffer_t delay_ring_buffer = {};

// Seconds of delay
float delay_time;

// Ratio of dry wet effect. 1 = full wet. 0 = full dry.
float dry_wet;

// Amount of signal to be fed back. 1 = all the out signal will be part of the new delayed samples. 0 = none of the out signal will be fed back
float feedback;

audio_sample getted_delayed_samples[1];
audio_sample delayed_samples_to_be_put[1];

void setup_processor(int sample_rate, float _delay_time, float _dry_wet, float _feedback)
{
  delay_time = _delay_time;
  dry_wet = _dry_wet;
  feedback = _feedback;

  // multiply by 2 to have enough space inside ring buffer to use the max delay time. if we reach max time limit we will have xruns.
  int ring_buffer_size = sample_rate * MAX_DELAY_TIME_IN_SEC * 2;
  ring_buffer_init(&delay_ring_buffer, ring_buffer_size, "delay");
  delay_ring_buffer.empty = false; // ring buffer already has silence, so for this case is not empty

  if (delay_time > MAX_DELAY_TIME_IN_SEC)
  {
    fprintf(stderr, "ERROR: max delay time in seconds is (%d)\n", MAX_DELAY_TIME_IN_SEC);
  }

  // forward head index
  delay_ring_buffer.head_index = delay_time * sample_rate;
}

void process_buffer(audio_sample *original_buffer, audio_sample *processed_buffer, int data_amount)
{
  for (int i = 0; i < data_amount; i++)
  {
    ring_buffer_get(&delay_ring_buffer, getted_delayed_samples, 1);
    processed_buffer[i] = ((float)original_buffer[i] * (1.0 - dry_wet) + (float)getted_delayed_samples[0] * dry_wet);

    delayed_samples_to_be_put[0] = ((float)original_buffer[i] + (float)getted_delayed_samples[0] * feedback);
    ring_buffer_put(&delay_ring_buffer, delayed_samples_to_be_put, 1);
  }
}
