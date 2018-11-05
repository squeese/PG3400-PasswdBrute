#include "talkiewalkie.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

const char TW_DEFAULT_UNIX_PATH[] = "/tmp/bcsocket";

static const char* TW_CODES[] = {
  "IDLE",
  "HASH",
  "WORK",
};

int tw_read_code(int socket_id, int* code) {
  static char buffer[5];
  int bytes = recv(socket_id, &buffer, 4, MSG_WAITALL);
  // int bytes = read(socket_id, &buffer, 4);
  *code = atoi(&buffer[1]);
  printf("%s %d READ %s %s\n", TW_LOG_PREFIX, socket_id, TW_CODES[*code], buffer);
  return bytes;
}

void tw_send_code(int socket_id, int code) {
  static char buffer[5];
  snprintf(buffer, 5, "[%02d]", code);
  printf("%s %d SEND %s %s\n", TW_LOG_PREFIX, socket_id, TW_CODES[code], buffer);
  send(socket_id, &buffer, 4, 0);
  // write(socket_id, &buffer, 4);
}

/*
int tw_read_number(int, socket_id, int* num) {
  static char buf[16];
  int bytes = recv(server, &buf, 16, MSG_WAITALL);
  *num = &atoi(&buf[1]);
  return bytes;
}

void tw_send_number(int socket_id, int num) {
  static char buffer[16];
}
*/