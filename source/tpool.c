#include "tpool.h"
#include <stdio.h>
#include <stdlib.h>
// #include <unistd.h>
#include <errno.h>
#include <string.h>
// #include <wait.h>
// #include <signal.h>

static const int signal_size = sizeof(struct tpool_signal);
static const char mqueue_in_path[] = "/tpoolinpath";
static const char mqueue_out_path[] = "/ptooloutpath";

void tpool_init(struct tpool* tp, int num) {
  tp->num_workers = num;
  tp->workers = calloc(num, sizeof(pthread_t));
  tp->provider = calloc(1, sizeof(pthread_t));
}

void tpool_provider_create(struct tpool* tp, tpool_handler_fn fn, void* arg) {
  tp->provider_fn = fn;
  pthread_create(tp->provider, NULL, fn, arg);
  pthread_detach(*tp->provider);
}

int tpool_provider_close(struct tpool* tp, tpool_handler_fn fn) {
  if (tp->provider_fn == fn) {
    pthread_cancel(*tp->provider);
    tp->provider_fn = NULL;
    return 1;
  }
  return 0;
}

void tpool_free(struct tpool* tp) {
  free(tp->workers);
  free(tp->provider);
}

int tpool_queue_init(struct tpool_queue* queue) {
  queue->signal_in_attr.mq_maxmsg = 3;
  queue->signal_out_attr.mq_maxmsg = 3; 
  queue->signal_in_attr.mq_msgsize = signal_size;
  queue->signal_out_attr.mq_msgsize = signal_size;
  mq_unlink(mqueue_in_path);
  mq_unlink(mqueue_out_path);
  queue->signal_in = mq_open(mqueue_in_path, O_CREAT | O_RDWR, 0666, &queue->signal_in_attr);
  queue->signal_out = mq_open(mqueue_out_path, O_CREAT | O_RDWR, 0666, &queue->signal_out_attr);
  if (queue->signal_in == -1 || queue->signal_out == -1) {
    fprintf(stderr, "Error opening messaging queue. errno(%d): %s\n", errno, strerror(errno));
    return -1;
  }
  return 0;
}

int tpool_queue_send(mqd_t slot, struct tpool_signal* tsignal, int prio) {
  return mq_send(slot, (const char*) tsignal, signal_size, prio);
}

int tpool_queue_read(mqd_t slot, struct tpool_signal* tsignal) {
  return mq_receive(slot, (char *) tsignal, signal_size, NULL);
}

int tpool_queue_free(struct tpool_queue* queue) {
  if (queue->signal_in != -1) {
    mq_close(queue->signal_in);
    mq_unlink(mqueue_in_path);
  }
  if (queue->signal_out != -1) {
    mq_close(queue->signal_out);
    mq_unlink(mqueue_out_path);
  }
  return 0;
}