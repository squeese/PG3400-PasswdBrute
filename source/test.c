#include "talkiewalkie.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <fcntl.h>

char TW_LOG_PREFIX[] = "<Test>";

int main(int argc, char** args) {
  // mkfifo("/tmp/bc_tmp_fifo", 0666);
  // int p[2];
  // p[0] = open("/tmp/bc_tmp_fifo", O_NONBLOCK | O_RDONLY);
  // p[1] = open("/tmp/bc_tmp_fifo", O_NONBLOCK | O_WRONLY);

  char* file = "/tmp/bcracksock";

  pid_t process_id = fork();
  if (process_id == 0) {
    // tw_send_code(p[1], TW_CODE_PING);
    // tw_send_code(p[1], TW_CODE_PONG);
    // tw_send_code(p[1], TW_CODE_IDLE);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));

    struct sockaddr_in addr2;
    memset(&addr2, 0, sizeof(addr2));

    printf("%ld %ld \n", sizeof(addr), sizeof(addr2));

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, file, sizeof(addr.sun_path)-1);
    unlink(file);
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
      perror("socket");
      exit(EXIT_FAILURE);
    }
    if (bind(sock, (struct sockaddr*) &addr, sizeof(addr)) != 0) {
      perror("bind");
      exit(EXIT_FAILURE);
    }
    if (listen(sock, 5) != 0) {
      perror("listen");
      exit(EXIT_FAILURE);
    }
    while (1) {
      int client = accept(sock, NULL, NULL);
      if (client == -1) {
        perror("accept");
        sleep(1);
        continue;
      }
      tw_send_code(client, TW_CODE_IDLE);
      tw_send_code(client, TW_CODE_PING);
      tw_send_code(client, TW_CODE_PONG);
      tw_read_code(client);
      close(client);
      break;
    }
    exit(EXIT_SUCCESS);
  } else {
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, file, sizeof(addr.sun_path)-1);
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
      perror("client socket");
      exit(EXIT_FAILURE);
    }
    while (1) {
      int status = connect(sock, (struct sockaddr*) &addr, sizeof(addr));
      if (status == -1) {
        perror("client connect, trying again");
        sleep(1);
        continue;
      }
      tw_read_code(sock);
      tw_read_code(sock);
      tw_read_code(sock);
      break;
    }
    close(sock);
    exit(EXIT_SUCCESS);
  }
}