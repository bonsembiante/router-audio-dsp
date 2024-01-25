#include "sound.h"
#include "ring_buffer.h"

/**
 * Init ring buffer
 */
void ring_buffer_init(ring_buffer_t *ring_buffer, int size, char *id)
{
  ring_buffer->id = id;
  ring_buffer->head_index = ring_buffer->tail_index = 0;
  ring_buffer->size = size;
  ring_buffer->empty = true;

  int size_in_bytes = size * sizeof(audio_sample);
  ring_buffer->buffer = (audio_sample *)malloc(size_in_bytes);
  memset(ring_buffer->buffer, SAMPLE_SILENCE, size_in_bytes);
}

/**
 * Destroy ring buffer
 */
void ring_buffer_destroy(ring_buffer_t *ring_buffer)
{
  free(ring_buffer->buffer);
}

/**
 * Get data from tail index, wrapping overflow
 */
void ring_buffer_get(ring_buffer_t *ring_buffer, audio_sample * output_buffer, int requested_amount)
{
  int available_data = ring_buffer_get_avail(ring_buffer);
  if(available_data < requested_amount){
    fprintf(stderr, "ringbuffer (%s) underrun\n", ring_buffer->id);
  }
  
  for (int i = 0; i < requested_amount; i++)
  {
    if (available_data < requested_amount && i >= available_data)
    {
      // Underrun: fill the rest of the buffer with silence
      output_buffer[i] = SAMPLE_SILENCE;
    }
    else
    {
      output_buffer[i] = ring_buffer->buffer[ring_buffer->tail_index++];
      ring_buffer->tail_index %= ring_buffer->size;
    }
  }
}

/**
 * Push data in head index, wrapping overflow
 */
void ring_buffer_put(ring_buffer_t *ring_buffer, audio_sample *data, int data_amount)
{
  int available_space = ring_buffer_put_avail(ring_buffer);
  if(available_space < data_amount){
    fprintf(stderr, "ringbuffer (%s) overrun\n", ring_buffer->id);
  }

  int iterable_spaces = available_space < data_amount ? available_space : data_amount;

  for (int i = 0; i < iterable_spaces; i++)
  {
    ring_buffer->buffer[ring_buffer->head_index++] = data[i];
    ring_buffer->head_index %= ring_buffer->size;
  }

  if (ring_buffer->empty)
  {
    // After first put, buffer is no longer empty
    ring_buffer->empty = false;
  }
}

/**
 * Check if there is enough amount of data between head and tail
 */
int ring_buffer_get_avail(ring_buffer_t *ring_buffer)
{
  if (ring_buffer->head_index == ring_buffer->tail_index)
  {
    return 0;
  }
  return (ring_buffer->size - ring_buffer->tail_index + ring_buffer->head_index) % ring_buffer->size;
}

/**
 * Check if there is enough space between tail and head
 */
int ring_buffer_put_avail(ring_buffer_t *ring_buffer)
{
  if (ring_buffer->tail_index == ring_buffer->head_index)
  {
    return ring_buffer->size;
  }
  return (ring_buffer->size - ring_buffer->head_index + ring_buffer->tail_index) % ring_buffer->size;
}