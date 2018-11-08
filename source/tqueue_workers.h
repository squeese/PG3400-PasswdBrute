#ifndef BC_THREAD_QUEUE_WORKERS_H
#define BC_THREAD_QUEUE_WORKERS_H
#include "tqueue.h"

enum {
  TQM_THREAD_STARTED          = (1 << 0),
  TQM_THREAD_STOPPED           = (1 << 1),
  TQM_THREAD_CLOSE         = (1 << 8),
  TQM_THREAD_FLUSH         

  TQM_WORD_DICTIONARY
  TQM_WORD_COMBINATOR
  TQM_WORD_ENCRYPT


  TQM_PROCESS_WORD

  TQM_TASK          = (1 << 2),
  TQM_WORD_LOADFILE   = (1 << 3),
  TQM_WORD_GENERATE   = (1 << 4),
  TQM_WORD_ENCRYPT        = (1 << 5),

  TQM_FOUND_PASSWORD      = (1 << 7),

  TQM_FLUSH         = (1 << 9),
  TQM_RETURN        = (1 << 10),
};

void tqueue_worker_root(mqd_t threads, mqd_t control, struct tqueue_message* msg);
void tqueue_worker_word_tester(mqd_t threads, mqd_t control, struct tqueue_message* msg);
void tqueue_worker_wdictionary(mqd_t threads, mqd_t control, struct tqueue_message* msg);
void tqueue_worker_wcombinator(mqd_t threads, mqd_t control, struct tqueue_message* msg);
void tqueue_worker_root_cleanup(void* arg);
void tqueue_worker_word_tester_cleanup(void* arg);
void tqueue_worker_wdictionary_cleanup(void* arg);
void tqueue_worker_wcombinator_cleanup(void* arg);
void tqueue_worker_flush(mqd_t threads, mqd_t control, struct tqueue_message* msg);

#endif