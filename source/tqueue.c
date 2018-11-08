#include "tqueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/*
  tqueue (thread queue), is an amalgam of <pthreads.h> and <mqueue.h>
  It's job is to help with the communication between threads, mostly
  between the main application thread and the spawned pthreads, but some
  in between.
  There are two channels, basicly input/output to/from the spawned pthreads
  and the main application thread.
*/

static const int MESSAGE_SIZE = sizeof(struct tqueue_message);
static const char MQUEUE_THREADS_PATH[] = "/tqueue_threads";
static const char MQUEUE_CONTROL_PATH[] = "/tqueue_control";

int tqueue_init(struct tqueue* tq) {
  tq->queue_threads_attr.mq_maxmsg = 3;
  tq->queue_control_attr.mq_maxmsg = 3; 
  tq->queue_threads_attr.mq_msgsize = MESSAGE_SIZE;
  tq->queue_control_attr.mq_msgsize = MESSAGE_SIZE;
  mq_unlink(MQUEUE_THREADS_PATH);
  mq_unlink(MQUEUE_CONTROL_PATH);
  tq->queue_threads = mq_open(MQUEUE_THREADS_PATH, O_CREAT | O_RDWR, 0666, &tq->queue_threads_attr);
  tq->queue_control = mq_open(MQUEUE_CONTROL_PATH, O_CREAT | O_RDWR, 0666, &tq->queue_control_attr);
  if (tq->queue_threads == -1 || tq->queue_control == -1) {
    fprintf(stderr, "Error opening messaging queue. errno(%d): %s\n", errno, strerror(errno));
    return -1;
  }
  return 0;
}

void tqueue_free(struct tqueue* tq) {
  if (tq->queue_threads != -1) {
    mq_close(tq->queue_threads);
    mq_unlink(MQUEUE_THREADS_PATH);
  }
  if (tq->queue_control != -1) {
    mq_close(tq->queue_control);
    mq_unlink(MQUEUE_CONTROL_PATH);
  }
}

void tqueue_unblock(struct tqueue* tq) {
  struct mq_attr attr;
  mq_getattr(tq->queue_threads, &attr);
  attr.mq_flags = O_NONBLOCK;
  mq_setattr(tq->queue_threads, &attr, NULL);
  mq_setattr(tq->queue_control, &attr, NULL);
}

static void tqueue_thread_cleanup(void *arg) {
  struct tqueue_context* ctx = arg;
  free(ctx);
}

static void* tqueue_thread_routine(void* arg) {
  pthread_cleanup_push(&tqueue_thread_cleanup, arg);
  struct tqueue_context* ctx = arg;
  struct tqueue_message msg = { 0 };
  ctx->fn(ctx->queue_threads, ctx->queue_control, &msg);
  pthread_cleanup_pop(1);
  return 0;
}

void tqueue_run(struct tqueue* tq, pthread_t* thread, tqueue_routine fn) {
  struct tqueue_context* ctx = malloc(sizeof(struct tqueue_context));
  ctx->queue_threads = tq->queue_threads;
  ctx->queue_control = tq->queue_control;
  ctx->fn = fn;
  pthread_create(thread, NULL, &tqueue_thread_routine, ctx);
}

int tqueue_read(mqd_t mqueue, struct tqueue_message* msg) {
  return mq_receive(mqueue, (char *) msg, MESSAGE_SIZE, NULL);
}

int tqueue_send(mqd_t mqueue, struct tqueue_message* msg, int flag, void* arg, int argn, int prio) {
  msg->flag = flag;
  msg->arg = arg;
  msg->argn = argn;
  return mq_send(mqueue, (const char*) msg, MESSAGE_SIZE, prio);
}