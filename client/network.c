#include "common.h"
#include "network.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

//#include <errno.h>
#include <netdb.h>

int 
net_connect(network_t* ctx, char* hostname, int port)
{
  int fd, ret;
  char portstr[6];
  struct addrinfo hints;
  struct addrinfo* servinfo;
  struct addrinfo* p;

  strncpy(ctx->host, hostname, HOST_NAME_MAX);
  ctx->host[HOST_NAME_MAX] = 0;
  ctx->port = port;

  snprintf(portstr, 6, "%d", port);

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

/*
int main(int argc, char *argv[])
{
  int sockfd, numbytes;  
  char buf[MAXDATASIZE];
  struct addrinfo hints, *servinfo, *p;
  int rv;
  char s[INET6_ADDRSTRLEN];

  if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
    perror("recv");
    exit(1);
  }

  buf[numbytes] = '\0';

  printf("client: received '%s'\n",buf);

  close(sockfd);

  return 0;
}
*/
