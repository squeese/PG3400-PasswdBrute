#define _GNU_SOURCE 1
#include "wpermutation.h"
#include "args.h"
#include "tpool.h"
#include "progress.h"
#include "wbuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <crypt.h>

struct tpool_queue queue;
struct args_client_config client_config;

int main() {
  struct progress prog;
  progress_init(&prog);
  progress_title(&prog, snprintf(prog.title, 64, "123456789012345678901234567890123456789012345678901234567890123"));
  progress_title(&prog, snprintf(prog.title, 64, "123456789012345678901234567890123456789012345678901234567890123"));
  progress_update(&prog, 50);
  printf("\n");


/*
  int len;
  char* prev = malloc(128 * sizeof(char));
  do {
    len = wbuffer_read(&wb);
    if (len <= 0) {
      printf("len=%d\n'%s'\n", len, prev);
      break;
    }
    memcpy(prev, wb.word, len);
  } while(1);
  */



  /*
  char* salt = "$1$ckvWM6T@$";
  char* pass = "9999";
  char* enc = crypt(pass, salt);
  printf("%s %s %s\n", salt, enc, pass);

  struct progress prog;
  progress_init(&prog);
  prog.max = 1000;
  progress_update(&prog, "ok", 1);
  write(1, prog.buffer, 96);

  progress_title(&prog, snprintf(prog.title, 64, "1234567890123456789012345678901234567890_______________________"));
  write(1, prog.buffer, 96);
  */


  // char buf[8];
  // memset(&buf, 'x', sizeof(buf));
  // write(1, buf, sizeof(buf));
  // fflush(stdout);

  // printf("\r");
  // fflush(stdout);

  // snprintf(buf, 8, "\r123456");
  // write(1, buf, sizeof(buf));
  // fflush(stdout);

  /*
  struct wpermutation wp;
  char* inputs = "abc";
  int length = 3;
  wperm_init(&wp, length, inputs, 3);
  wperm_update(&wp, 0);
  int size = 13;

  char* buffer = malloc(size * sizeof(char));
  int c = size / (sizeof(char) * (length + 1));
  int l;
  while ((l = wperm_generate(&wp, buffer, c)) > 0) {
    for (int i = 0; i < l; i++) {
      if (*(buffer + i) == 0) {
        *(buffer + i) = '_';
      }
    }
    printf("%d `%s`\n", l, buffer);
  }
  */

  return 0;
}