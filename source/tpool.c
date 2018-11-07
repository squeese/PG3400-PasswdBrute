#include "tpool.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

static const int message_size = sizeof(struct tpool_message);
static const char mqueue_threads_path[] = "/tpoolinpath";
static const char mqueue_control_path[] = "/ptooloutpath";

int tpool_init(struct tpool* tp) {
  tp->count = 8;
  tp->threads = NULL; // calloc(8, sizeof(pthread_t*));
  tp->queue_threads_attr.mq_maxmsg = 3;
  tp->queue_control_attr.mq_maxmsg = 3; 
  tp->queue_threads_attr.mq_msgsize = message_size;
  tp->queue_control_attr.mq_msgsize = message_size;
  mq_unlink(mqueue_threads_path);
  mq_unlink(mqueue_control_path);
  tp->queue_threads = mq_open(mqueue_threads_path, O_CREAT | O_RDWR, 0666, &tp->queue_threads_attr);
  tp->queue_control = mq_open(mqueue_control_path, O_CREAT | O_RDWR, 0666, &tp->queue_control_attr);
  if (tp->queue_threads == -1 || tp->queue_control == -1) {
    fprintf(stderr, "Error opening messaging queue. errno(%d): %s\n", errno, strerror(errno));
    return -1;
  }
  return 0;
}

void tpool_free(struct tpool* tp) {
  if (tp->queue_threads != -1) {
    mq_close(tp->queue_threads);
    mq_unlink(mqueue_threads_path);
  }
  if (tp->queue_control != -1) {
    mq_close(tp->queue_control);
    mq_unlink(mqueue_control_path);
  }
}

static void tpool_thread_cleanup(void *arg) {
  struct tpool_context* ctx = arg;
  pthread_mutex_destroy(&ctx->lock);
  // pthread_cond_destroy(&ctx->cond);
  free(ctx);
}

static void* tpool_thread_routine(void* arg) {
  pthread_cleanup_push(&tpool_thread_cleanup, arg);
  struct tpool_context* ctx = arg;
  struct tpool_message msg;
  memset(&msg, 0, sizeof(msg));
  ctx->fn(ctx->tp, &msg, &ctx->lock);
  pthread_cleanup_pop(1);
  pthread_exit(0);
}

struct tpool_context* tpool_run(struct tpool* tp, tpool_routine fn) {
  struct tpool_context* ctx = calloc(1, sizeof(struct tpool_context));
  ctx->tp = tp;
  ctx->fn = fn;
  pthread_mutex_init(&ctx->lock, NULL);
  // pthread_cond_init(&ctx->cond, NULL);
  pthread_create(&ctx->thread, NULL, &tpool_thread_routine, ctx);
  // pthread_detach(ctx->thread);
  return ctx;
}

void tpool_cancel(struct tpool_context* ctx) {
  pthread_mutex_lock(&ctx->lock);
  // pthread_cond_wait(&ctx->cond, &ctx->lock);
  pthread_cancel(ctx->thread);
  pthread_mutex_unlock(&ctx->lock);
}

int tpool_read(mqd_t mqueue, struct tpool_message* msg) {
  mq_receive(mqueue, (char *) msg, message_size, NULL);
  return msg->flag;
}

int tpool_send(mqd_t mqueue, struct tpool_message* msg, int flag, void* arg, int prio) {
  msg->flag = flag;
  msg->arg = arg;
  return mq_send(mqueue, (const char*) msg, message_size, prio);
}