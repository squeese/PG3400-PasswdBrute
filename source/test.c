#define _GNU_SOURCE 1
// #include "wpermutation.h"
// #include "args.h"
// #include "tpool.h"
// #include "progress.h"
// #include "wbuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <crypt.h>
#include <pthread.h>
#include <mqueue.h>
          
// struct tpool_queue queue;
// struct args_client_config client_config;

/*
static pthread_mutex_t lock;
static pthread_cond_t condition;

void cleanupfn(void* arg) {
  printf("%s CLEANUP\n", (char*) arg);
}
*/

void* syncfn(void* arg) {
  printf("%s thread\n", (char*) arg);
  sleep(1);
  printf("%s exiting\n", (char*) arg);
  pthread_exit(arg);
}

int main() {
  /*
  pthread_mutex_init(&lock, NULL);
  pthread_cond_init(&condition, NULL);

  */
  pthread_t a;
  pthread_create(&a, NULL, &syncfn, "a");
  pthread_t b;
  pthread_create(&b, NULL, &syncfn, "b");

  printf("join a\n");
  pthread_join(a, NULL);
  printf("after a\n");

  sleep(1);
  printf("join b\n");
  pthread_join(b, NULL);
  printf("after b\n");



  /*
  sleep(1);
  printf("main broadcast\n");
  pthread_mutex_lock(&lock);
  pthread_cond_broadcast(&condition);
  pthread_mutex_unlock(&lock);

  sleep(1);
  printf("main broadcast\n");
  pthread_mutex_lock(&lock);
  pthread_cond_broadcast(&condition);
  pthread_mutex_unlock(&lock);

  sleep(1);
  printf("main cancel\n");
  pthread_cancel(a);
  pthread_cancel(b);
  sleep(1);
  */
  
  // pthread_create(&b, NULL, &syncfn, "b");

/*
  sleep(1);
  pthread_cond_broadcast(&condition);
  pthread_mutex_unlock(&lock);

  sleep(2);
  pthread_mutex_lock(&lock);
  pthread_cond_broadcast(&condition);
  sleep(1);
  pthread_mutex_unlock(&lock);

  sleep(3);
  */



  /*
  struct progress prog;
  progress_init(&prog);
  progress_title(&prog, snprintf(prog.title, 64, "123456789012345678901234567890123456789012345678901234567890123"));
  progress_title(&prog, snprintf(prog.title, 64, "123456789012345678901234567890123456789012345678901234567890123"));
  progress_update(&prog, 50);
  printf("\n");


  struct wbuffer wb;
  wbuffer_init(&wb, "misc/myspace.txt", NULL);
  int stride = 300;
  int n = stride;
  int len;
  int max = 0;
  do {
    len = wbuffer_word(&wb, 500);
    if (len <= 0) break;
    max = (len > max) ? len : max;
    printf("`%s` (%d)\n", wb.word, len);
    n--;
    if (n == 0) {
      // sleep(1);
      n = stride;
    }
  } while(1);
  printf("max: %d\n", max);



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
/*
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 500

// https://linux.die.net/man/3/getaddrinfo

int main(int argc, char *argv[]) {
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int sfd, s;

  if (argc < 3) {
    fprintf(stderr, "Usage: %s host port msg...\n", argv[0]);
    return -1;
  }

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  s = getaddrinfo(argv[1], argv[2], &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(EXIT_FAILURE);
  }

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    printf("ok \n");
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sfd == -1) continue;
    if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) break;
    close(sfd);
  }

  if (rp == NULL) {
    fprintf(stderr, "Could not connect\n");
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(result);
  exit(EXIT_SUCCESS);
}
*/
