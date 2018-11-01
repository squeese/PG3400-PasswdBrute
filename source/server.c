#include "args.h"
#include "talkiewalkie.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <wait.h>
#include <pthread.h>
// #include <sys/types.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/un.h>

char TW_LOG_PREFIX[] = "<Server>";
// static void* server_state_accept(void*);
static void* server_state_idle(int);

int main(int argc, char** args) {
  int server_socket;
  struct args_server_config server_config;
  struct sockaddr* server_address;
  int server_address_size;

  // Create config with input arguments
  if (args_server_init(&server_config, argc, args) != 0) {
    return EXIT_FAILURE;
  }

  if (argc == 0) {
    // Creating UNIX socket
    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1) {
      fprintf(stderr, "Error opening UNIX socket. errno(%d): %s\n", errno, strerror(errno));
      exit(EXIT_FAILURE);
    }
    // Creating UNIX socket address
    static char* path = "/tmp/bcsocket";
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
    unlink(path);
    server_address = (struct sockaddr*) &addr;
    server_address_size = sizeof(addr);
    printf("<Server> UNIX socket %s %d\n", path, server_address_size);
  } else {
    // Creating INET socket (using commandline configs)
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
      fprintf(stderr, "Error opening INET socket. errno(%d): %s\n", errno, strerror(errno));
      args_server_free(&server_config);
      exit(EXIT_FAILURE);
    }
    // Creating INET socket address
    struct sockaddr_in addr;
    // memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = server_config.port;
    addr.sin_addr.s_addr = (server_config.host == NULL)
      ? INADDR_ANY
      : inet_addr(server_config.host);
    server_address = (struct sockaddr*) &addr;
    server_address_size = sizeof(addr);
    args_server_free(&server_config);
  }

  // Binding socket to address
  if (bind(server_socket, server_address, server_address_size) != 0) {
    fprintf(stderr, "Error binding socket. errno(%d): %s\n", errno, strerror(errno));
    close(server_socket);
    exit(EXIT_FAILURE);
  }
  printf("<Server> socket bound %d\n", server_socket);

  // Listen for connections
  if (listen(server_socket, 5) != 0) {
    fprintf(stderr, "Error listening on socket. errno(%d): %s\n", errno, strerror(errno));
    close(server_socket);
    exit(EXIT_FAILURE);
  }
  printf("<Server> socket listening %d\n", server_socket);


  // Attemt to accept incoming client connections
  // Only accepting one client at a time, dont see the need
  // to support multiple users at the same time
  // pthread_t thread;
  // pthread_create(&thread, NULL, server_state_accept, &server_socket);
  // pthread_join(thread, NULL);

  int client_socket = accept(server_socket, NULL, NULL);
  if (client_socket == -1) {
    fprintf(stderr, "Error accepting client connection. errno(%d): %s\n", errno, strerror(errno));
    close(server_socket);
    exit(EXIT_FAILURE);
  }
  printf("<Server> client (%d) connected\n", client_socket);

  // start talkiewalkie state loop
  tw_state_fn fn = &server_state_idle;
  tw_send_code(client_socket, TW_CODE_IDLE);
  while (fn != NULL) {
    fn = (tw_state_fn)(*fn)(client_socket);
  }
  printf("<Server> client (%d) disconnected\n", client_socket);

  close(client_socket);
  close(server_socket);
  printf("<Server> exiting.\n");
  return EXIT_SUCCESS;
}

/*
static void* server_state_accept(void* arg) {
  int server_socket = *(int*) arg;
  int client_socket;
  struct sockaddr_in client_address;
  socklen_t client_address_size = sizeof(struct sockaddr_in);
  while (1) {
    printf("??\n");
    client_socket = accept(server_socket, (struct sockaddr*) &client_address, &client_address_size);
    printf("!!\n");
    if (client_socket == -1) {
      fprintf(stderr, "Error accepting client connection. errno(%d): %s\n", errno, strerror(errno));
      close(server_socket);
      pthread_exit(0);
      // return EXIT_FAILURE;
    }
    printf("<Server> client (%s) connected\n", inet_ntoa(client_address.sin_addr));

    // start talkiewalkie state loop
    tw_state_fn fn = &server_state_idle;
    tw_send_code(client_socket, TW_CODE_IDLE);
    while (fn != NULL) {
      fn = (tw_state_fn)(*fn)(client_socket);
    }

    // close client connection
    printf("<Server> client (%s) disconnected\n", inet_ntoa(client_address.sin_addr));
    close(client_socket);
    break;
  }
  pthread_exit(0);
}
*/

static void* server_state_idle(int client_socket) {
  int code = tw_read_code(client_socket);
  switch (code) {
    case TW_CODE_PING:
      tw_send_code(client_socket, TW_CODE_PONG);
      tw_send_code(client_socket, TW_CODE_IDLE);
      break;
    default:
      printf("<Server> IDLE: unhandled code %d\n", code);
  }
  return &server_state_idle;
}