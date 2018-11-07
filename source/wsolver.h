#ifndef BC_WORD_SOLVER_H
#define BC_WORD_SOLVER_H
#include "args.h"
#include "tpool.h"
#include <stdio.h>

extern struct args_client_config client_config;

struct wsolver_work {
  char* buffer;
  int length;
  char* pass;
};

int wsolver_thread_worker(struct tpool*, struct tpool_message*, pthread_mutex_t*);

#endif