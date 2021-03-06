#ifndef NETWORK_H
#define NETWORK_H

#include <limits.h>
#include <pthread.h>

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 64
#endif

#define NET_KEEPALIVE_ENABLED
#define NET_KEEPALIVE_SECS    120

#define NET_READ_TIMEOUT_USEC 10000
#define NET_MAX_BYTES         8191 // should match device.h:DVAP_MSG_MAX_BYTES

// net_rx_fptr is a function pointer that takes two arguments,
// a pointer to a buffer and the length of the buffer
typedef void (*net_rx_fptr)(unsigned char* buf, int buf_bytes);

typedef struct {  
  net_rx_fptr callback; 		// pointer to rx callback

  char host[HOST_NAME_MAX + 1];
  int port;

  int fd;

  int try_restart;			// if true restart network on timeout
  int shutdown;         		// set true to shut down rx loop
  pthread_mutex_t shutdown_mutex;	// acquire before using shutdown

  pthread_mutex_t tx_mutex;		// acquire before writing to network
  pthread_t keepalive_thread;		// pthread associated with keepalive

  pthread_t rx_thread;			// pthread associated with read loop
} network_t;

int net_init(network_t* ctx, char* hostname, int port, net_rx_fptr callback);
int net_connect(network_t* ctx);
void net_wait(network_t* ctx);
int net_read(network_t* ctx, char* msg_type, unsigned char* buf, int buf_bytes);
int net_write(network_t* ctx, unsigned char* buf, int buf_bytes);
void net_stop(network_t* ctx, int try_restart);

int net_should_shutdown(network_t* ctx);
void* net_keepalive_loop(void* arg);
void* net_read_loop(void* arg);

#endif
