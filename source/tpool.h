#ifndef BC_TPOOL_H
#define BC_TPOOL_H
#include <pthread.h>
#include <mqueue.h>

enum {
  TPOOL_WSOLVER_WORK = 1,
  TPOOL_WDICTIONARY_WORK = 2,
  TPOOL_WCOMBINATIONS_WORK = 3,
  TPOOL_THREAD_LOAD = 4,
  TPOOL_THREAD_EXIT = 5,
};

struct tmp_work {
  char* buffer;
  int length;
};

struct tpool {
  struct mq_attr queue_threads_attr;
  struct mq_attr queue_control_attr;
  mqd_t queue_threads;
  mqd_t queue_control;
  pthread_t* threads;
  int count;
};

struct tpool_message {
  int flag;
  void* arg;
};

typedef int (*tpool_routine)(struct tpool*, struct tpool_message*, pthread_mutex_t* lock);

struct tpool_context {
  struct tpool* tp;
  tpool_routine fn;
  pthread_t thread;
  pthread_mutex_t lock;
  // pthread_cond_t cond;
};


int tpool_init(struct tpool*);
void tpool_free(struct tpool*);
struct tpool_context* tpool_run(struct tpool*, tpool_routine);
int tpool_read(mqd_t, struct tpool_message*);
int tpool_send(mqd_t, struct tpool_message*, int, void*, int);
void tpool_cancel(struct tpool_context*);

// void tpool_provider_create(struct tpool*, tpool_handler_fn, void*);
// int tpool_provider_close(struct tpool*, tpool_handler_fn);
// int tpool_queue_init(struct tpool_queue*);
// int tpool_queue_send(mqd_t, struct tpool_signal*, int);
// int tpool_queue_read(mqd_t, struct tpool_signal*);
// int tpool_queue_free(struct tpool_queue*);

#endif
