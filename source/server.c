#include "config.h"
#include "talkiewalkie.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

void* state_idle(int socket_id) {
  tw_read(socket_id);
  printf("server : idle\n");
  return NULL;
}

void* state_connecting(int socket_id) {
  tw_send(socket_id, TW_CODE_OPEN);
  printf("server > %d\n", TW_CODE_OPEN);
  return &state_idle;
}

void* cfg_initial(void* config, char* arg);

void* cfg_host(void* config, char* arg) {
  ((struct sockaddr_in*) config)->sin_addr.s_addr = inet_addr(arg);
  return &cfg_initial;
}

void* cfg_port(void* config, char* arg) {
  ((struct sockaddr_in*) config)->sin_port = atoi(arg);
  return &cfg_initial;
}

void* cfg_initial(void* config, char* arg) {
  if (cfg_match(arg, 2, "-h", "--host")) return &cfg_host;
  if (cfg_match(arg, 2, "-p", "--port")) return &cfg_port;
  return NULL;
}

int main(int argc, char** args) {
  int sock_server = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_server == -1) {
    printf("Error (%d): %s\n", errno, strerror(errno));
    return 0;
  }

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = 0;
  address.sin_addr.s_addr = INADDR_ANY;
  cfg_parse(&address, argc, args, &cfg_initial);

  if (bind(sock_server, (struct sockaddr*) &address, sizeof(address)) != 0) {
    printf("Error (%d): %s\n", errno, strerror(errno));
    close(sock_server);
    return 0;
  }

  unsigned int address_size = sizeof(address);
  getsockname(sock_server, (struct sockaddr*) &address, &address_size);
  printf("{host:%d, port:%d}\n", address.sin_addr.s_addr, address.sin_port);

  if (listen(sock_server, 5) != 0) {
    printf("Error (%d): %s\n", errno, strerror(errno));
    close(sock_server);
    return 0;
  }

  int sock_client = accept(sock_server, NULL, NULL);
  if (sock_client == -1) {
    printf("Error (%d): %s\n", errno, strerror(errno));
    close(sock_server);
    return 0;
  }

  tw_state_fn fn = &state_connecting;
  while (fn != NULL) {
    fn = (tw_state_fn)(*fn)(sock_client);
  }

  printf("Closing down server.\n");
  close(sock_server);
  return 0;
}
