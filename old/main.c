#include "config.h"
#include "talkiewalkie.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// #include <time.h>
// #include <assert.h>

void* state_connecting(int socket_id) {
  tw_send(socket_id, TW_CODE_OPEN);
  printf("client > %d\n", TW_CODE_OPEN);
  return NULL;
}


int main(int argc, char** args) {
  struct cfg_client cfg = { NULL, 52666, NULL, "misc/small.txt", 4, 32, 4 };
  cfg_parse(&cfg, argc, args, &cfg_client_root);

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
  if (cfg.host == NULL) {
		// there is not specified a remote server, so we launch
		// one localy on this machine instead
		pid_t server_pid = fork();
		if (server_pid == 0) {
			static char *server_args[] = { "", "-p", "52666" };
			execv("server", server_args);
			exit(EXIT_SUCCESS);
		}
  }

  int client_socket = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in client_address;
  client_address.sin_family = AF_INET;
  client_address.sin_port = cfg.port;
  client_address.sin_addr.s_addr = (cfg.host == NULL) ? INADDR_ANY : inet_addr(cfg.host);

	for (int status, i = 0;; i++) {
		status = connect(client_socket, (struct sockaddr*) &client_address, sizeof(client_address));
		if (status == 0) break;
		if (i > 5) {
			printf("Unable to connect to server\n");
			return 0;
		}
		printf("retry\n");
		sleep(1);
	}
	printf("yas!\n");

  tw_state_fn fn = &state_connecting;
  while (fn != NULL) {
    fn = (tw_state_fn)(*fn)(client_socket);
  }

/*
  tw_state_fn fn = &state_connecting;
  while (fn != NULL) {
    fn = (tw_state_fn)(*fn)(nsocket);
  }
	*/

	wait(NULL);
  return 0;
}

/*

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