#ifndef BC_TPOOL_H
#define BC_TPOOL_H
#include <pthread.h>

struct tpool {
  pthread_t* threads;
  unsigned int count;
};

typedef void*(*tpool_fn)(void*);

void tpool_init(struct tpool*, unsigned int);

char* tpool_run(struct tpool*, tpool_fn, void*);

void tpool_free(struct tpool*);

#endif
