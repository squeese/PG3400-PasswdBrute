#include "tpool.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

void tpool_init(struct tpool* tp, unsigned int count) {
  tp->count = count;
  tp->threads = malloc(count * sizeof(pthread_t));
}

void tpool_stop(struct tpool* tp) {
  for (unsigned int i = 0; i < tp->count; i++) {
    if (pthread_cancel(tp->threads[i]) != 0) {
      printf("ERROR %d %d %s\n", i, errno, strerror(errno));
    }
  }
}

struct args {
  void*(*fn)(void*, unsigned int*);
  void* arg;
  unsigned int* alive;
};

void* dispatch(void* arg) {
  struct args* args = arg;
  return args->fn(args->arg, args->alive);
}

int tpool_start(struct tpool* tp, void* arg, void* (*fn)(void*, unsigned int*)) {
  printf("<tpool_start>\n");
  unsigned int alive = 1;
  struct args args;
  args.fn = fn;
  args.arg = arg;
  args.alive = &alive;
  for (unsigned int i = 0; i < tp->count; i++) {
    // printf("<tpool_%d>\n", i);
    // pthread_create(&tp->threads[i], NULL, cb, arg);
    pthread_create(&tp->threads[i], NULL, dispatch, &args);
  }
  for (unsigned int i = 0; i < tp->count; i++) {
    void* result = NULL;
    pthread_join(tp->threads[i], &result);
    printf("<tpool:%d> returned %s\n", i, (char*) result);
    // alive = 0;
    // printf("</tpool_%d>\n", i);
  }
  printf("</tpool_start>\n");
  return EXIT_SUCCESS;
}

/*
struct solver_arg {
  solver fn;
  void* arg;
  unsigned int tid;
  char* salt;
  char* hash;
};

static void* solver_dispatcher(void* arg) {
  struct solver_arg* sarg = arg;
  return sarg->fn(sarg->arg, sarg->tid, sarg->salt, sarg->hash);
}
*/

void tpool_run_solver(struct tpool* tp, solver_fn fn, void* state) {
  for (unsigned int i = 0; i < tp->count; i++) {
    pthread_create(&tp->threads[i], NULL, fn, state);
  }
  for (unsigned int i = 0; i < tp->count; i++) {
    pthread_join(tp->threads[i], NULL);
  }
}

void tpool_free(struct tpool* tp) {
  free(tp->threads);
}