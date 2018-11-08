#include "args.h"
#include "tqueue.h"
#include "tqueue_workers.h"
#include "wdictionary.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <mqueue.h>
#include <time.h>
          
struct args_client_config client_config;

static pthread_t a;
static mqd_t queue;


void* rootfn(void* arg) {
  int y = 0;
  struct timespec tm;
  clock_gettime(CLOCK_REALTIME, &tm);
  tm.tv_sec += 1;
  int v = mq_timedsend(queue, (const char*) &y, sizeof(int), 0, &tm);
  printf("thread also sent: %d\n", v);
  return 0;
}

int main() {
  mq_unlink("/testqueue");
  queue = mq_open("/testqueue", O_CREAT | O_RDWR, 0666, NULL);
  int x = 0;
  for (int i = 0; i < 10; i++) {
    mq_send(queue, (const char*) &x, sizeof(int), 0);
    printf("sent %d\n", i);
  }

  pthread_create(&a, NULL, rootfn, NULL);

/*
  printf("setattr\n");
  struct mq_attr attr;
  attr.mq_flags = O_NONBLOCK;
  mq_setattr(queue, &attr, NULL);

  printf("take one value\n");
  mq_receive(queue, (char*) &x, sizeof(int), NULL);

*/

  pthread_join(a, NULL);
  printf("exit\n");
  return 0;
}