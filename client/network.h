#ifndef NETWORK_H
#define NETWORK_H

#include <limits.h>
#include <pthread.h>

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 64
#endif

#define NET_MAX_SIZE 8191	// should match device.h:DVAP_MSG_MAX_BYTES

typedef struct {  
  char host[HOST_NAME_MAX + 1];
  int port;

  int fd;

  int shutdown;
  pthread_mutex_t shutdown_mutex;

  pthread_t rx_thread;
} network_t;

int net_init(network_t* ctx, char* hostname, int port);
int net_connect(network_t* ctx);
void net_wait(network_t* ctx);
void net_stop(network_t* ctx);

int should_shutdown(network_t* ctx);
void* net_read_loop(void* arg);

#endif
