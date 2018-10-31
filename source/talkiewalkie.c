#include "talkiewalkie.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

extern char TW_LOG_PREFIX[];

static const char* TW_CODES[] = {
  "IDLE",
  "EXIT",
  "PING",
  "PONG",
};

int tw_read_code(int socket_id) {
  static char buffer[5];
  // recv(socket_id, &buffer, 4, 0);
  read(socket_id, &buffer, 4);
  int code = atoi(&buffer[1]);
  printf("%s READ %s %s\n", TW_LOG_PREFIX, TW_CODES[code], buffer);
  return code;
}

void tw_send_code(int socket_id, int code) {
  static char buffer[5];
  snprintf(buffer, 5, "[%02d]", code);
  printf("%s SEND %s %s\n", TW_LOG_PREFIX, TW_CODES[code], buffer);
  // send(socket_id, &buffer, 4, 0);
  write(socket_id, &buffer, 4);
}