#ifndef BC_TPOOL_H
#define BC_TPOOL_H
#include <pthread.h>
#include <mqueue.h>

typedef void*(*tpool_handler_fn)(void*);

enum {
  SIGNAL_WORKER_EXIT = 2,
  SIGNAL_PROVIDER_START = 4,
  SIGNAL_WORKLOAD_START = 8,
  SIGNAL_WORKLOAD_COMPLETED = 16,
  SIGNAL_DICTIONARY_COMPLETED = 32,
  SIGNAL_PERMUTATION_COMPLETED = 64,
  SIGNAL_REMOTE_CONNECTED = 128,
  SIGNAL_REMOTE_DISCONNECTED = 256,

  SIGNAL_CONNECTION_SUCCESS = 512,
  SIGNAL_CONNECTION_FAILED = 1024,
  SIGNAL_CONNECTION_DROPPED = 2048,
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
  pthread_mutex_t lock;
  pthread_cond_t condition;
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
