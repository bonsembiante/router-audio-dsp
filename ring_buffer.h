#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include "sound.h"

typedef struct
{
  int head_index;
  int tail_index;
  audio_sample * buffer;
  int size;
  bool empty;
  char * id;
} ring_buffer_t;

void ring_buffer_init(ring_buffer_t *ring_buffer, int size, char *id);
void ring_buffer_destroy(ring_buffer_t *ring_buffer);
void ring_buffer_get(ring_buffer_t *ring_buffer, audio_sample * output_buffer, int requested_amount);
void ring_buffer_put(ring_buffer_t *ring_buffer, audio_sample *data, int data_amount);
int ring_buffer_get_avail(ring_buffer_t * ring_buffer);
int ring_buffer_put_avail(ring_buffer_t * ring_buffer);

#endif