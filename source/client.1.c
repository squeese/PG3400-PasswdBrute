#include "config.h"
// #include "talkiewalkie.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <sys/wait.h>
// #include <netinet/in.h>
// #include <unistd.h>
// #include <assert.h>

struct client_config {
  char* host;
  int port;
  char* hash;
  char* dictionary;
  int threads;
  int stride;
  int depth;
};

void* client_config_initial(void* cfg, char* arg);

void* client_config_dictionary(void* cfg, char* arg) {
  ((struct client_config*) cfg)->dictionary = arg;
  return &client_config_initial;
}

void* client_config_threads(void* cfg, char* arg) {
  ((struct client_config*) cfg)->threads = atoi(arg);
  return &client_config_initial;
}

void* client_config_stride(void* cfg, char* arg) {
  ((struct client_config*) cfg)->stride = atoi(arg);
  return &client_config_initial;
}

void* client_config_depth(void* cfg, char* arg) {
  ((struct client_config*) cfg)->depth = atoi(arg);
  return &client_config_initial;
}

void* client_config_host(void* cfg, char* arg) {
  ((struct client_config*) cfg)->host = arg;
  return &client_config_initial;
}

void* client_config_port(void* cfg, char* arg) {
  ((struct client_config*) cfg)->port = atoi(arg);
  return &client_config_initial;
}

void* client_config_initial(void* cfg, char* arg) {
  if (config_match(arg, 2, "-d", "--dictionary")) return &client_config_dictionary;
  if (config_match(arg, 2, "-t", "--threads")) return &client_config_threads;
  if (config_match(arg, 2, "-s", "--stride")) return &client_config_stride;
  if (config_match(arg, 2, "-d", "--depth")) return &client_config_depth;
  if (config_match(arg, 2, "-h", "--host")) return &client_config_host;
  if (config_match(arg, 2, "-p", "--port")) return &client_config_port;
  if (strlen(arg) == 34) ((struct client_config*) cfg)->hash = arg;
  else printf("Unknown config parameter: %s\n", arg);
  return NULL;
}

int main(int argc, char** args) {
  struct client_config cfg = { NULL, 0, NULL, "misc/small.txt", 4, 32, 4 };
  config_parse(&cfg, argc, args, &client_config_initial);

  printf("\nConfig\n");
  printf(" host       : %s\n", cfg.host);
  printf(" port       : %d\n", cfg.port);
  printf(" dictionary : %s\n", cfg.dictionary);
  printf(" threads    : %d\n", cfg.threads);
  printf(" stride     : %d\n", cfg.stride);
  printf(" depth      : %d\n", cfg.depth);
  printf(" hash       : %s\n", cfg.hash);
  printf("\n");

  // create a connection to server
  if (cfg.host != NULL) {
    // create local server
  }

  return 0;
}

/*
  int nsocket;
  nsocket = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = portnr();
  address.sin_addr.s_addr = INADDR_ANY;

  int status = connect(nsocket, (struct sockaddr*) &address, sizeof(address));
  if (status != 0) {
    printf("Unable to connect to server\n");
    return 0;
  }

  tw_state_fn fn = &state_connecting;
  while (fn != NULL) {
    fn = (tw_state_fn)(*fn)(nsocket);
  }

  printf("Closing connection.\n");
  close(nsocket);
  int pipefd[2];
  char buf;
  pipe(pipefd);
  pid_t pid_server = fork();
  if (pid_server == 0) {
    // static char *argv[] = { "9003" };
    // execv("./server", argv);
    close(pipefd[1]);
    char msg[10];
    int i = 0;
    while (read(pipefd[0], &buf, 1) > 0) {
      // write(1, &buf, 1);
      msg[i++] = buf;
    }
    msg[i++] = '\n';
    write(1, &msg, i);
    // write(1, "x", 1);
    close(pipefd[0]);
    exit(EXIT_SUCCESS);
  } else {
    // waitpid(pid, 0, 0);
    close(pipefd[0]);
    char msg[] = "hello";
    write(pipefd[1], msg, strlen(msg));
    close(pipefd[1]);
    wait(NULL);
    exit(EXIT_SUCCESS);
  }
  */