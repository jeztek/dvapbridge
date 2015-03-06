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
net_init(network_t* ctx, char* hostname, int port, net_rx_fptr callback)
{
  ctx->callback = callback;

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

int
net_write(network_t* ctx, unsigned char* buf, int buf_bytes)
{
  return send(ctx->fd, buf, buf_bytes, 0);
}

int
net_read(network_t* ctx, char* msg_type, unsigned char* buf, int buf_bytes)
{
  if (!ctx) return -1;
  return packet_read(ctx->fd, msg_type, buf, buf_bytes);
}

void
net_stop(network_t* ctx)
{
  pthread_mutex_lock(&(ctx->shutdown_mutex));
  ctx->shutdown = TRUE;
  pthread_mutex_unlock(&(ctx->shutdown_mutex));
}

int
net_should_shutdown(network_t* ctx)
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

  fd_set set;
  char msg_type;
  int ret;
  unsigned char buf[NET_MAX_BYTES];

  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = NET_READ_TIMEOUT_USEC;

  while(!net_should_shutdown(ctx)) {
    FD_ZERO(&set);
    FD_SET(ctx->fd, &set);
    ret = select(ctx->fd + 1, &set, NULL, NULL, &timeout);
    if (ret < 0) {
      fprintf(stderr, "Error waiting for data from network\n");
      return NULL;
    }
    else if (ret == 0) {
      //debug_print("%s\n", "network select timeout");
      continue;
    }

    ret = net_read(ctx, &msg_type, buf, NET_MAX_BYTES);
    if (ret < 0) {
      fprintf(stderr, "Error reading from network\n");
      return NULL;
    }
    else if (ret == 0) {
      fprintf(stderr, "Timeout while reading from network\n");
      return NULL;
    }

    hex_dump("net rx", buf, ret);
    (ctx->callback)(buf, ret);
  }

  return NULL;
}
