#include "args.h"
#include "talkiewalkie.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <wait.h>

char TW_LOG_PREFIX[] = "<Client>";
static void* client_state_idle(int);
static void* client_state_ping(int);

int main(int argc, char** args) {
  struct args_client_config client_config;
  struct sockaddr* client_address;
  int client_address_size;

  // Create config with input arguments
  if (args_client_init(&client_config, argc, args) != 0) {
    return EXIT_FAILURE;
  }

  // Check if we need to start a local server if there isnt one
  // specified in the configs
  if (client_config.servers == NULL) {
    pid_t process_id = fork();
    if (process_id == 0) {
      static char* args[] = {};
      execv("./server", args);
      fprintf(stderr, "Error forking child server process. errno(%d): %s\n", errno, strerror(errno));
      exit(EXIT_FAILURE);
    }
    // Create a new server entry in the config for the local child server process.
    static char* path = "/tmp/bcsocket";
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
    int server = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server == -1) {
      fprintf(stderr, "Error creating socket. errno(%d): %s\n", errno, strerror(errno));
      exit(EXIT_FAILURE);
    }
    client_config.num_servers = 1;
    client_config.servers = malloc(sizeof(int));
    *(client_config.servers) = server;
    printf("<Client> UNIX socket %s\n", path);
    client_address = (struct sockaddr*) &addr;
    client_address_size = sizeof(addr);
  }

  // Attempt to make a connection to server
  printf("<Client> connecting to servers %d\n", *(client_config.servers));
  for (int attempts = 4; attempts >= 0; attempts--) {
    int status = connect(*(client_config.servers), client_address, client_address_size);
    if (status != -1) break;
    if (attempts <= 0) {
      fprintf(stderr, "Error connecting to server %d. errno(%d): %s\n", *(client_config.servers), errno, strerror(errno));
      exit(EXIT_FAILURE);
    }
    sleep(1);
    printf("<Client> retrying server %d\n", *(client_config.servers));
  }
  printf("<Client> connected to server %d\n", *(client_config.servers));

  // Start the TALKIEWALKIE state loop
  tw_state_fn fn = &client_state_idle;
  while (fn != NULL) {
    fn = (tw_state_fn)(*fn)(*(client_config.servers));
  }

  // Cleanups
  close(*(client_config.servers));
  args_client_free(&client_config);
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