#ifndef NETWORK_H
#define NETWORK_H

#include <limits.h>
#include <pthread.h>

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 64
#endif

#define NET_READ_TIMEOUT_USEC 10000
#define NET_MAX_BYTES         8191 // should match device.h:DVAP_MSG_MAX_BYTES

typedef void (*net_rx_fptr)(unsigned char* buf, int buf_bytes);

typedef struct {  
  net_rx_fptr callback;

  char host[HOST_NAME_MAX + 1];
  int port;

  int fd;

  int try_restart;	// if true, restart network connection on timeout
  int shutdown;         // if true, stop net_read_loop
  pthread_mutex_t shutdown_mutex;

  pthread_t rx_thread;
} network_t;

int net_init(network_t* ctx, char* hostname, int port, net_rx_fptr callback);
int net_connect(network_t* ctx);
void net_wait(network_t* ctx);
int net_read(network_t* ctx, char* msg_type, unsigned char* buf, int buf_bytes);
int net_write(network_t* ctx, unsigned char* buf, int buf_bytes);
void net_stop(network_t* ctx);

int net_should_shutdown(network_t* ctx);
void* net_read_loop(void* arg);

#endif
