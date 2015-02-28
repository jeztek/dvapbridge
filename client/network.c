#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>

#include "common.h"
#include "network.h"

int
net_init(network_t* ctx, char* hostname, int port)
{
  strncpy(ctx->host, hostname, HOST_NAME_MAX);
  ctx->host[HOST_NAME_MAX] = 0;
  ctx->port = port;

  ctx->shutdown = FALSE;
  pthread_mutex_init(&(ctx->shutdown_mutex), NULL);

  if (!net_connect(ctx)) {
    return FALSE;
  }

  pthread_create(&(ctx->rx_thread), NULL, net_read_loop, ctx);
  return TRUE;
}

int 
net_connect(network_t* ctx)
{
  int fd, ret;
  char portstr[6];
  struct addrinfo hints;
  struct addrinfo* servinfo;
  struct addrinfo* p;

  snprintf(portstr, 6, "%d", ctx->port);

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((ret = getaddrinfo(ctx->host, portstr, &hints, &servinfo)) != 0) {
    fprintf(stderr, "net_connect - %s\n", gai_strerror(ret));
    return FALSE;
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      continue;
    }
    if (connect(fd, p->ai_addr, p->ai_addrlen) == -1) {
      close(fd);
      continue;
    }

    break;
  }

  if (p == NULL) {
    return FALSE;
  }
  ctx->fd = fd;
  freeaddrinfo(servinfo);

  return TRUE;
}

void
net_wait(network_t* ctx)
{
  if (!ctx) return;
  pthread_join(ctx->rx_thread, NULL);
  close(ctx->fd);

  pthread_mutex_destroy(&(ctx->shutdown_mutex));
}

void
net_stop(network_t* ctx)
{
  pthread_mutex_lock(&(ctx->shutdown_mutex));
  ctx->shutdown = TRUE;
  pthread_mutex_unlock(&(ctx->shutdown_mutex));
}

int
should_shutdown(network_t* ctx)
{
  int shutdown;
  if (!ctx) return TRUE;

  pthread_mutex_lock(&(ctx->shutdown_mutex));
  shutdown = ctx->shutdown;
  pthread_mutex_unlock(&(ctx->shutdown_mutex));
  return shutdown;
}

void*
net_read_loop(void* arg)
{
  network_t* ctx = (network_t *)arg;

  int ret;
  unsigned char buf[100];

  fd_set set;
  struct timeval timeout;
  int rv;

  timeout.tv_sec = 0;
  timeout.tv_usec = 10000;

  while(!should_shutdown(ctx)) {
    FD_ZERO(&set);
    FD_SET(ctx->fd, &set);

    rv = select(ctx->fd + 1, &set, NULL, NULL, &timeout);
    if (rv < 0) {
      fprintf(stderr, "net_read_loop error\n");
    }
    else if (rv == 0) {
      //fprintf(stderr, "net_read_loop timeout\n");
    }
    else {
      ret = read(ctx->fd, &buf, 100);
      hex_dump("net rx", buf, ret);
    }
  }

  return NULL;
}
