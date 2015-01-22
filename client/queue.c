// A thread-safe fifo queue that stores byte arrays
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "queue.h"

void
queue_init(queue_t* q)
{
  q->first = 0;
  q->last = QUEUE_SIZE - 1;
  q->count = 0;

  pthread_mutex_init(&(q->mutex), NULL);
  pthread_cond_init(&(q->empty), NULL);
  pthread_cond_init(&(q->fill), NULL);
}

int
queue_insert(queue_t* q, unsigned char* data, int len)
{
  if (len > QUEUE_ENTRY_SIZE) {
    fprintf(stderr, "Error: queue insert fail, data too large\n");
    return FALSE;
  }

  pthread_mutex_lock(&(q->mutex));
  while(q->count >= QUEUE_SIZE) {
    pthread_cond_wait(&(q->empty), &(q->mutex));
  }

  q->last = (q->last + 1) % QUEUE_SIZE;
  memcpy(&(q->data[q->last]), data, len);
  q->data_len[q->last] = len;
  q->count += 1;

  pthread_cond_signal(&(q->fill));
  pthread_mutex_unlock(&(q->mutex));

  return TRUE;
}

int
queue_remove(queue_t* q, unsigned char* data, int* len)
{
  int l;

  pthread_mutex_lock(&(q->mutex));
  while (q->count <= 0) {
    pthread_cond_wait(&(q->fill), &(q->mutex));
  }

  l = q->data_len[q->first];
  memcpy(data, &(q->data[q->first]), l);
  *len = l;
  q->first = (q->first + 1) % QUEUE_SIZE;
  q->count -= 1;

  pthread_cond_signal(&(q->empty));
  pthread_mutex_unlock(&(q->mutex));

  return TRUE;
}

int
queue_is_empty(queue_t* q)
{
  int count;

  pthread_mutex_lock(&(q->mutex));
  count = q->count;
  pthread_mutex_unlock(&(q->mutex));

  if (count <= 0) return TRUE;
  return FALSE;
}

int
queue_is_full(queue_t* q)
{
  int count;

  pthread_mutex_lock(&(q->mutex));
  count = q->count;
  pthread_mutex_unlock(&(q->mutex));

  if (count >= QUEUE_SIZE) return TRUE;
  return FALSE;
}
