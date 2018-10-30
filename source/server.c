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
  int code = tw_read(socket_id);
  printf("code: %d\n", code);
  return NULL;
}

void* state_connecting(int socket_id) {
  tw_send(socket_id, TW_CODE_OPEN);
  return &state_idle;
}

void* config_initial(void* config, char* arg);

void* config_host(void* config, char* arg) {
  ((struct sockaddr_in*) config)->sin_addr.s_addr = inet_addr(arg);
  return &config_initial;
}

void* config_port(void* config, char* arg) {
  ((struct sockaddr_in*) config)->sin_port = atoi(arg);
  return &config_initial;
}

void* config_initial(void* config, char* arg) {
  if (config_match(arg, 2, "-h", "--host")) return &config_host;
  if (config_match(arg, 2, "-p", "--port")) return &config_port;
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
  config_parse(&address, argc, args, &config_initial);

  if (bind(sock_server, (struct sockaddr*) &address, sizeof(address)) != 0) {
    printf("Error (%d): %s\n", errno, strerror(errno));
    close(sock_server);
    return 0;
  }

  unsigned int address_size = sizeof(address);
  getsockname(sock_server, (struct sockaddr*) &address, &address_size);
  printf("{host:%d, port:%d}\n", address.sin_addr.s_addr, address.sin_port);
  FILE *f = fopen(".port", "w+");
  fprintf(f, "%d", address.sin_port);
  fclose(f);

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
