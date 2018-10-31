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
    int process_pipe[2];
    pipe(process_pipe);
    pid_t process_id = fork();
    if (process_id == 0) {
      // This is the child server process.
      // Rewire the stdout to our custom pipe, so we can communicate.
      // We need to get the PORT nr that the OS will assign to the server.
      close(1); 
      dup(process_pipe[1]);
      close(process_pipe[0]);
      close(process_pipe[1]);
      static char* args[] = {};
      execv("./server", args);
      fprintf(stderr, "Error forking child server process. errno(%d): %s\n", errno, strerror(errno));
      exit(EXIT_FAILURE);
    } else {
      // Read from the pipe (child server process' stdout), to get the PORT nr
      char buffer[16];
      printf("ok go\n");
      int bytes = read(process_pipe[0], buffer, 7);
      fprintf(stderr, "ok go\n");
      fprintf(stdout, "ok go\n");
      buffer[bytes] = 0;
      // Cleanup/close the pipes, since we dont need them anymore
      close(process_pipe[0]);
      close(process_pipe[1]);
      // Create a new server entry in the config for the local child server process.
      struct sockaddr_in* server_address = malloc(sizeof(struct sockaddr_in));
      server_address->sin_family = AF_INET;
      server_address->sin_port = atoi(buffer);
      server_address->sin_addr.s_addr = INADDR_ANY;
      client_config.num_servers = 1;
      client_config.servers = server_address;
    }
  }

  printf("%s:%d\n", inet_ntoa(client_config.servers->sin_addr), client_config.servers->sin_port);


/*
  int fd[2];
  pipe(fd);
  pid_t pid = fork();
  if (pid == 0) {
    close(1);     // close stdout
    dup(fd[1]);
    close(fd[0]);
    close(fd[1]);
    static char *args[] = {};
    execv("./test", args);
  } else {
    printf("main\n");
    char buffer[16];
    for (int i = 0; i < 16; i++) {
      buffer[i] = '_';
    }
    read(fd[0], buffer, 15);
    buffer[15] = 0;
    printf("buffer: '%s'\n", buffer);
    close(fd[0]);
    close(fd[1]);
    waitpid(pid, NULL, 0);
    exit(EXIT_SUCCESS);
  }
  */

  /*
  // Attempt to create a socket
  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1) {
    fprintf(stderr, "Error opening socket. errno(%d): %s\n", errno, strerror(errno));
    args_client_free(&client_config);
    return EXIT_FAILURE;
  }

  // Apply the input config to socket configuration
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = ARGS_DEFAULT_SERVER_PORT;
  server_address.sin_addr.s_addr = INADDR_ANY;
 
  // Attempt to make a connection to server
  printf("<Client> connecting to %s\n", inet_ntoa(server_address.sin_addr));
  for (int attempts = 4; attempts >= 0; attempts--) {
    int status = connect(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));
    if (status != -1) break;
    if (attempts <= 0) {
      fprintf(stderr, "Error connecting socket to address %s:%d. errno(%d): %s\n", inet_ntoa(server_address.sin_addr), server_address.sin_port, errno, strerror(errno));
      return EXIT_FAILURE;
    }
    sleep(1);
    printf("<Client> retry %s\n", inet_ntoa(server_address.sin_addr));
  }
  printf("<Client> connected to %s\n", inet_ntoa(server_address.sin_addr));

  // start talkiewalkie state loop
  tw_state_fn fn = &client_state_idle;
  while (fn != NULL) {
    fn = (tw_state_fn)(*fn)(server_socket);
  }

  close(server_socket);
  */
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