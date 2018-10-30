#include "talkiewalkie.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int tw_read(int socket_id) {
  static char buffer[8];
  int i = recv(socket_id, &buffer[0], 8, 0);
  printf("read(%d): %s -> %d\n", i, buffer, atoi(&buffer[1]));
  return atoi(&buffer[1]);
}

void tw_send(int socket_id, int code) {
  static char buffer[8];
  sprintf(&buffer[0], "[%d]", code);
  // printf("write(%ld): %s <- %d\n", strlen(buffer), buffer, code);
  send(socket_id, &buffer[0], strlen(buffer), 0);
}