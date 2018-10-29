#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

int main() {
  int nsocket;
  nsocket = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(9002); // 0 getsockname();
  address.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0

  int status = connect(nsocket, (struct sockaddr*) &address, sizeof(address));
  if (status != 0) {
    printf("Unable to connect to server\n");
    return 0;
  }
  printf("Connected to server\n");

  char response[256];
  recv(nsocket, &response, sizeof(response), 0);
  printf("Response is: %s\n", response);

  close(nsocket);

  return 0;
}

/*
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