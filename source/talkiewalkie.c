#include "talkiewalkie.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int tw_read_code(int socket_id, void* prefix) {
  static char buffer[8];
  int i = recv(socket_id, &buffer[0], 8, 0);
  printf("%s tw_read_code(%d): %s -> %d\n", (char*) prefix, i, buffer, atoi(&buffer[1]));
  return atoi(&buffer[1]);
}

void tw_send_code(int socket_id, int code, void* prefix) {
  static char buffer[8];
  sprintf(&buffer[0], "[%d]", code);
  printf("%s write(%ld): %s <- %d\n", (char*) prefix, 8, buffer, code);
  send(socket_id, &buffer[0], 8, 0);
}