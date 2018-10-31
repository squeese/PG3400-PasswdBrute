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

char TW_LOG_PREFIX[] = "<Client>";
static void* client_state_idle(int);
static void* client_state_ping(int);

int main(int argc, char** args) {
  // Create config with input arguments
  struct args_client_config client_config;
  if (args_client_init(&client_config, argc, args) != 0) {
    return EXIT_FAILURE;
  }
  printf("\n");
  printf("<Client> -d %s (dictionary)\n", client_config.dictionary);
  printf("<Client> -l %d (length)\n", client_config.length);
  printf("<Client>    %s (salt)\n", client_config.salt);
  printf("<Client>    %s (hash)\n", client_config.hash);
  args_client_free(&client_config);

  // Check if we need to start a local server if there isnt one
  // specified in the configs
  if (client_config.servers == NULL) {
    pid_t process_id = fork();
    if (process_id == 0) {
      static char* args[] = {};
      execv("./server", args);
      fprintf(stderr, "Error forking child server process. errno(%d): %s\n", errno, strerror(errno));
      exit(EXIT_FAILURE);
    } else {
      // Create a new server entry in the config for the local child server process.
      struct sockaddr_in* server_address = malloc(sizeof(struct sockaddr_in));
      server_address->sin_family = AF_INET;
      server_address->sin_port = ARGS_DEFAULT_SERVER_PORT;
      server_address->sin_addr.s_addr = INADDR_ANY;
      client_config.num_servers = 1;
      client_config.servers = server_address;
    }
  }

  // printf("%s:%d\n", inet_ntoa(client_config.servers->sin_addr), client_config.servers->sin_port);

  // Attempt to create socket
  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1) {
    fprintf(stderr, "Error opening socket. errno(%d): %s\n", errno, strerror(errno));
    args_client_free(&client_config);
    return EXIT_FAILURE;
  }

  // Attempt to make a connection to server
  printf("<Client> connecting to %s:%d\n", inet_ntoa(client_config.servers->sin_addr), client_config.servers->sin_port);
  for (int attempts = 4; attempts >= 0; attempts--) {
    int status = connect(server_socket, (struct sockaddr*) client_config.servers, sizeof(client_config.servers));
    if (status != -1) break;
    if (attempts <= 0) {
      fprintf(stderr, "Error connecting socket to address %s:%d. errno(%d): %s\n", inet_ntoa(client_config.servers->sin_addr), client_config.servers->sin_port, errno, strerror(errno));
      return EXIT_FAILURE;
    }
    sleep(1);
    printf("<Client> retry %s:%d\n", inet_ntoa(client_config.servers->sin_addr), client_config.servers->sin_port);
  }
  printf("<Client> connected to %s\n", inet_ntoa(client_config.servers->sin_addr));

  // start talkiewalkie state loop
  tw_state_fn fn = &client_state_idle;
  while (fn != NULL) {
    fn = (tw_state_fn)(*fn)(server_socket);
  }

  close(server_socket);
  printf("<Client> exiting\n");
  return EXIT_SUCCESS;
}

static void* client_state_idle(int client_socket) {
  int code = tw_read_code(client_socket);
  if (code != TW_CODE_IDLE) {
    printf("Client> ERROR: expected IDLE\n");
    return NULL;
  }
  sleep(2);
  return &client_state_ping;
}

static void* client_state_ping(int client_socket) {
  tw_send_code(client_socket, TW_CODE_PING);
  int code = tw_read_code(client_socket);
  if (code != TW_CODE_PONG) {
    printf("<Client> ERROR: expected PONG\n");
    return NULL;
  }
  return &client_state_idle;
}