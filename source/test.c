#include "args.h"
#include "wbuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <wait.h>
#include <pthread.h>
#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <crypt.h>

static struct args_client_config client_config;

int main(int argc, char** args) {
  args_client_init(&client_config, argc, args);
  args_client_free(&client_config);

  int size = 400;

  struct wbuffer wb;
  wbuffer_init(&wb, client_config.dictionary);
  char* buffer = malloc(sizeof(char) * size);

  // int s = wbuffer_read(&wb);
  // printf("(%d) '%s'\n", s, wb.word);

  int l;
  while ((l = wbuffer_fill(&wb, buffer, size)) > 0) {
    printf("------------\n");
    printf("(%d) %s\n", l, buffer);
  }

  free(buffer);
  return 0;
}