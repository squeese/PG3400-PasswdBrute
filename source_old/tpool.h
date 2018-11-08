#ifndef BC_tqueue_H
#define BC_tqueue_H
#include <pthread.h>

struct tpool {
  pthread_t* threads;
  unsigned int count;
};

typedef void*(*tqueue_fn)(void*);

void tqueue_init(struct tpool*, unsigned int);

char* tqueue_run(struct tpool*, tqueue_fn, void*);

void tqueue_free(struct tpool*);

#endif
