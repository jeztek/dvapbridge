#ifndef QUEUE_H
#define QUEUE_H

#define QUEUE_SIZE	 10
#define QUEUE_ENTRY_SIZE 8191

typedef struct {
  char data[QUEUE_SIZE + 1][QUEUE_ENTRY_SIZE];
  int data_len[QUEUE_SIZE + 1];
  int first;
  int last;
  int count;
} queue_t;

void queue_init(queue_t* q);
void queue_delete(queue_t* q);
int queue_insert(queue_t* q, unsigned char* data, int len);
int queue_remove(queue_t* q, unsigned char* data, int* len);
int queue_empty(queue_t* q);

#endif 
