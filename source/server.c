#include "args.h"
#include "talkiewalkie.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
// #include <sys/types.h>
// #include <netinet/in.h>

static char TW_LOG_PREFIX[] = "<Server>";
static void* server_state_idle(int);

int main(int argc, char** args) {
  // Create config with input arguments
  struct args_server_config server_config;
  if (args_server_init(&server_config, argc, args) != 0) {
    return EXIT_FAILURE;
  }
  printf("\n");
  printf("<Server> -t %s (host)\n", server_config.host == NULL ? "localhost" : server_config.host);
  printf("<Server> -p %d (port)\n", server_config.port);
  printf("<Server> -t %d (threads)\n", server_config.threads);

  // Attempt to create a server socket
  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1) {
    fprintf(stderr, "Error opening socket. errno(%d): %s\n", errno, strerror(errno));
    args_server_free(&server_config);
    return EXIT_FAILURE;
  }

  // Apply the input config to socket configuration
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = server_config.port;
  server_address.sin_addr.s_addr = (server_config.host == NULL)
    ? INADDR_ANY
    : inet_addr(server_config.host);

  // Dont need to keep the server_config's hostname around, just free it now
  args_server_free(&server_config);

  // Attempt to bind socket to specified address/port
  if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) != 0) {
    fprintf(stderr, "Error binding socket to address %s:%d. errno(%d): %s\n", inet_ntoa(server_address.sin_addr), server_address.sin_port, errno, strerror(errno));
    close(server_socket);
    return EXIT_FAILURE;
  }
  printf("<Server> socket bound to %s:%d\n", inet_ntoa(server_address.sin_addr), server_address.sin_port);

  // Attempt to make socket listen for clients connecting
  if (listen(server_socket, 1) != 0) {
    fprintf(stderr, "Error listening on socket. errno(%d): %s\n", errno, strerror(errno));
    close(server_socket);
    return EXIT_FAILURE;
  }
  printf("<Server> socket listening\n");

  // Attemt to accept incoming client connections
  // Only accepting one client at a time, dont see the need
  // to support multiple users at the same time
  int client_socket;
  struct sockaddr_in client_address;
  socklen_t client_address_size = sizeof(struct sockaddr_in);
  while (1) {
    client_socket = accept(server_socket, (struct sockaddr*) &client_address, &client_address_size);
    if (client_socket == -1) {
      fprintf(stderr, "Error accepting client connection. errno(%d): %s\n", errno, strerror(errno));
      close(server_socket);
      return EXIT_FAILURE;
    }
    printf("<Server> client (%s) connected\n", inet_ntoa(client_address.sin_addr));
    // start talkiewalkie state loop
    tw_state_fn fn = &server_state_idle;
    while (fn != NULL) {
      fn = (tw_state_fn)(*fn)(client_socket);
    }
    // close client connection
    printf("<Server> client (%s) disconnected\n", inet_ntoa(client_address.sin_addr));
    close(client_socket);
  }

  printf("<Server> exiting.\n");
  close(server_socket);
  return EXIT_SUCCESS;
}


static void* server_state_idle(int client_socket) {
  tw_send_code(client_socket, TW_CODE_IDLE, &TW_LOG_PREFIX);
  unsigned int code;
  while (1) {
    code = tw_read_code(client_socket, &TW_LOG_PREFIX);
    switch (code) {
      case TW_CODE_PING:
        tw_send_code(client_socket, TW_CODE_PONG, &TW_LOG_PREFIX);
        break;
      case TW_CODE_EXIT:
        return NULL;
      default:
        printf("<Server> unknown code recieved: %d\n", code);
    }
  }
  return NULL;
}