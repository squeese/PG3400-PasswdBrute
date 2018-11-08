#ifndef BC_THREAD_QUEUE_H
#define BC_THREAD_QUEUE_H
#include <pthread.h>
#include <mqueue.h>

struct tqueue {
  struct mq_attr queue_threads_attr;
  struct mq_attr queue_control_attr;
  mqd_t queue_threads;
  mqd_t queue_control;
};

// struct used to pass values between threads with mqueue.h
struct tqueue_message {
  int flag;
  void* arg;
  int argn;
};

typedef void (*tqueue_routine)(mqd_t, mqd_t, struct tqueue_message*);

struct tqueue_context {
  mqd_t queue_threads;
  mqd_t queue_control;
  tqueue_routine fn;
};

int tqueue_init(struct tqueue*);
void tqueue_unblock(struct tqueue*);
void tqueue_run(struct tqueue*, pthread_t* thread, tqueue_routine);
int tqueue_read(mqd_t, struct tqueue_message*);
int tqueue_send(mqd_t, struct tqueue_message*, int, void*, int, int);
void tqueue_free(struct tqueue*);

#endif
