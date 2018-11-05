#ifndef BC_CLIENT_THANDLERS_H
#define BC_CLIENT_THANDLERS_H
#include "tpool.h"
#include "args.h"

// arf
struct thread_cleanup_t {
  char** buffer;
  void* arg;
};

extern struct args_client_config client_config;
extern struct tpool_queue queue;

void* thread_encrypt_worker_local(void*);
void* thread_encrypt_worker_remote(void*);
void* thread_words_from_dictionary_provider(void*);
void* thread_words_from_permutation_provider(void*);
void* thread_issue_close_signal(void*);

#endif
