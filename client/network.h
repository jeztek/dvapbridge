#ifndef NETWORK_H
#define NETWORK_H

#include <limits.h>

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 64
#endif

#define NET_MAX_SIZE 8191	// should match device.h:DVAP_MSG_MAX_BYTES

typedef struct {  
  char host[HOST_NAME_MAX + 1];
  int port;

  int fd;

} network_t;

int net_connect(network_t* ctx, char* hostname, int port);

#endif
