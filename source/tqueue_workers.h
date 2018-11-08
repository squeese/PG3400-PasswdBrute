#ifndef BC_THREAD_QUEUE_WORKERS_H
#define BC_THREAD_QUEUE_WORKERS_H
#include "tqueue.h"

enum {
  TQMESSAGE_PUSH          = (1 << 0),
  TQMESSAGE_POP           = (1 << 1),
  TQMESSAGE_NEXT          = (1 << 2),
  TQMESSAGE_WDICTIONARY   = (1 << 3),
  TQMESSAGE_WCOMBINATOR   = (1 << 4),
  TQMESSAGE_TEST_WORDS    = (1 << 5),
  TQMESSAGE_PASSWORD      = (1 << 6),
  TQMESSAGE_CLOSE         = (1 << 7),
  TQMESSAGE_RETURN        = (1 << 8),
};

struct test_words {
  char* buffer;
  int length;
};

void tqueue_worker_root(mqd_t threads, mqd_t control, struct tqueue_message* msg);
void tqueue_worker_word_tester(mqd_t threads, mqd_t control, struct tqueue_message* msg);
void tqueue_worker_wdictionary(mqd_t threads, mqd_t control, struct tqueue_message* msg);
void tqueue_worker_flush(mqd_t threads, mqd_t control, struct tqueue_message* msg);
void tqueue_worker_close(mqd_t threads, mqd_t control, struct tqueue_message* msg);
void tqueue_worker_root_cleanup(void* arg);
void tqueue_worker_word_tester_cleanup(void* arg);
void tqueue_worker_wdictionary_cleanup(void* arg);

#endif