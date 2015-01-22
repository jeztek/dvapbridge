#include <stdio.h>
#include <string.h>
#include "queue.h"

int main(int argc, char* argv[])
{
  queue_t q;
  unsigned char buf0[8];
  unsigned char buf1[10];
  int len;

  queue_init(&q);
  printf("queue empty? ");
  if (queue_is_empty(&q)) {
    printf("yes\n");
  }
  else {
    printf("no\n");
  }

  memcpy(buf0, "hello!", 7);
  printf("buf0: %s\n", buf0);

  queue_insert(&q, buf0, 7);
  memcpy(buf0, "hi!", 4);
  queue_insert(&q, buf0, 7);
  memcpy(buf0, "beep", 5);
  queue_insert(&q, buf0, 7);
  memcpy(buf0, "boop", 5);
  queue_insert(&q, buf0, 7);
  queue_insert(&q, buf0, 7);
  queue_insert(&q, buf0, 7);
  queue_insert(&q, buf0, 7);
  queue_insert(&q, buf0, 7);
  queue_insert(&q, buf0, 7);
  printf("q size: %d\n", q.count);

  queue_insert(&q, buf0, 7);
  printf("q size: %d\n", q.count);

  queue_remove(&q, buf1, &len);
  printf("remove: %s [%d]\n", buf1, len);
  printf("q size: %d\n", q.count);

  queue_remove(&q, buf1, &len);
  printf("remove: %s [%d]\n", buf1, len);
  printf("q size: %d\n", q.count);

  queue_remove(&q, buf1, &len);
  printf("remove: %s [%d]\n", buf1, len);
  printf("q size: %d\n", q.count);

  queue_remove(&q, buf1, &len);
  printf("remove: %s [%d]\n", buf1, len);
  printf("q size: %d\n", q.count);

  queue_remove(&q, buf1, &len);
  printf("remove: %s [%d]\n", buf1, len);
  printf("q size: %d\n", q.count);

  queue_remove(&q, buf1, &len);
  printf("remove: %s [%d]\n", buf1, len);
  printf("q size: %d\n", q.count);

  queue_remove(&q, buf1, &len);
  printf("remove: %s [%d]\n", buf1, len);
  printf("q size: %d\n", q.count);

  queue_remove(&q, buf1, &len);
  printf("remove: %s [%d]\n", buf1, len);
  printf("q size: %d\n", q.count);

  queue_remove(&q, buf1, &len);
  printf("remove: %s [%d]\n", buf1, len);
  printf("q size: %d\n", q.count);

  queue_remove(&q, buf1, &len);
  printf("remove: %s [%d]\n", buf1, len);
  printf("q size: %d\n", q.count);

  return 0;
}
