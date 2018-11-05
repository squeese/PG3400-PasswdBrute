#ifndef BC_TPOOL_H
#define BC_TPOOL_H
#include <pthread.h>
#include <mqueue.h>

typedef void*(*tpool_handler_fn)(void*);

enum {
  SIGNAL_WORKER_EXIT,
  SIGNAL_WORKLOAD_START,
  SIGNAL_WORKLOAD_COMPLETE,
  SIGNAL_DICTIONARY_COMPLETE,
  SIGNAL_PERMUTATION_COMPLETE,
};

struct tpool_signal {
  int flag;
  void *arg;
};

struct tpool_work {
  char* salt;
  char* hash;
  char* pass;
  char* buffer;
  int length;
};

struct tpool_queue {
  struct mq_attr signal_in_attr;
  struct mq_attr signal_out_attr;
  mqd_t signal_in;
  mqd_t signal_out;
};

struct tpool {
  int num_workers;
  pthread_t* workers;
  pthread_t* provider;
  tpool_handler_fn provider_fn;
};

void tpool_init(struct tpool*, int);
void tpool_provider_create(struct tpool*, tpool_handler_fn, void*);
int tpool_provider_close(struct tpool*, tpool_handler_fn);
void tpool_free(struct tpool*);

int tpool_queue_init(struct tpool_queue*);
int tpool_queue_send(mqd_t, struct tpool_signal*, int);
int tpool_queue_read(mqd_t, struct tpool_signal*);
int tpool_queue_free(struct tpool_queue*);

#endif
