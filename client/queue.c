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
}

int
queue_insert(queue_t* q, unsigned char* data, int len)
{
  if (q->count >= QUEUE_SIZE) {
    fprintf(stderr, "Error: queue full\n");
    return FALSE;
  }
  if (len > QUEUE_ENTRY_SIZE) {
    fprintf(stderr, "Error: queue insert fail, data too large\n");
    return FALSE;
  }

  q->last = (q->last + 1) % QUEUE_SIZE;
  memcpy(&(q->data[q->last]), data, len);
  q->data_len[q->last] = len;
  q->count += 1;
  return TRUE;
}

int
queue_remove(queue_t* q, unsigned char* data, int* len)
{
  int l;
  if (q->count <= 0) {
    return FALSE;
  }
  l = q->data_len[q->first];
  memcpy(data, &(q->data[q->first]), l);
  *len = l;
  q->first = (q->first + 1) % QUEUE_SIZE;
  q->count -= 1;
  return TRUE;
}

int
queue_empty(queue_t* q)
{
  if (q->count <= 0) return TRUE;
  return FALSE;
}
