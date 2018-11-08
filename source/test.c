// #include "wpermutation.h"
// #include "args.h"
// #include "tpool.h"
// #include "progress.h"
// #include "wbuffer.h"
#include "args.h"
#include "tqueue.h"
#include "tqueue_workers.h"
#include "wdictionary.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
// #include <crypt.h>
#include <pthread.h>
#include <mqueue.h>
          
struct args_client_config client_config;

/*
static pthread_t a;

void cleanupfn(void* arg) {
  printf("and after sleep, cleanup\n");
}

void* defered(void* arg) {
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  for (int i = 0; i < 3; i++) {
    printf("...sleeping\n");
    sleep(1);
  }
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_testcancel();
  printf("this shouldnt be called\n");
  return 0;
}

void* rootfn(void* arg) {
  pthread_cleanup_push(&cleanupfn, NULL);
  defered(arg);
  printf("this shouldnt be called either\n");
  pthread_cleanup_pop(0);
  pthread_exit(0);
}


void* cancelfn(void *arg) {
  pthread_mutex_t* t = arg;
  printf("B aquire lock, I should wait 2 seconds\n");
  pthread_mutex_lock(t);
  printf("B cancel A\n");
  pthread_cancel(a);
  printf("B cancelled A\n");
  pthread_mutex_unlock(t);
  pthread_exit(0);
}
*/

int main() {
  printf("tqueue.h \n");
  printf("tqueue_workers.h \n");
  printf("wdictionary.h \n");
  /*
  pthread_create(&a, NULL, &rootfn, NULL);
  sleep(1);
  printf("Issue cancel\n");
  pthread_cancel(a);
  pthread_join(a, NULL);
  printf("After cancel\n");
  */
  return 0;
}