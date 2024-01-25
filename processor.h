#ifndef PROCESSOR_H
#define PROCESSOR_H

#define MAX_DELAY_TIME_IN_SEC 2

#include "sound.h"

void setup_processor(int sample_rate, float _delay_time, float _dry_wet, float _feedback);
void process_buffer(audio_sample *original_buffer, audio_sample *processed_buffer, int data_amount);

#endif