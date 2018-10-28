#ifndef BC_QUEUE_H
#define BC_QUEUE_H
#define _GNU_SOURCE 1

#include "dictionary.h"
#include <pthread.h>
#include <crypt.h>

enum {
  QUEUE_THREADS = 16,
  QUEUE_JOBS = 64,
  QUEUE_STRIDE = 256,
};

struct job {
  int num;
  char** words;
  unsigned int offset;
  char* target;
};

struct queue {
  pthread_mutex_t lock;
  pthread_t* threads;
  struct dictionary* dict;
  struct job** jobs;
  unsigned int dict_cursor;
  unsigned int job_cursor;
  char salt[13];
  char* hash;
};

int queue_init(struct queue*, struct dictionary*, char* hash);
void queue_start(struct queue*);
struct job* queue_pop_job(struct queue*);
void queue_free_job(struct job*);
// void queue_stop(struct job*);
void queue_free(struct queue*);

#endif
