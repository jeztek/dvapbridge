// A thread-safe fifo queue that stores byte arrays
#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>

#define QUEUE_SIZE	 10
#define QUEUE_ENTRY_SIZE 8191	// should match device.h:DVAP_MSG_MAX_BYTES

typedef struct {
  unsigned char data[QUEUE_SIZE + 1][QUEUE_ENTRY_SIZE];
  int data_len[QUEUE_SIZE + 1];
  int first;
  int last;
  int count;

  pthread_mutex_t mutex;
  pthread_cond_t empty;
  pthread_cond_t fill;
} queue_t;

void queue_init(queue_t* q);
void queue_delete(queue_t* q);

// NOTE: This call will block if the queue is full
int queue_insert(queue_t* q, unsigned char* data, int len);

// NOTE: This call will block if the queue is empty
int queue_remove(queue_t* q, unsigned char* data, int* len);

int queue_is_empty(queue_t* q);
int queue_is_full(queue_t* q);

#endif 
