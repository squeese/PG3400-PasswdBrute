#include "args.h"
#include "client_thandlers.h"
#include "tpool.h"
#include "wbuffer.h"
#include "wpermutation.h"
#include <stdio.h>
#include <stdlib.h>

struct args_client_config client_config;
struct tpool_queue queue;

int main(int argc, char** args) {
  // Create config with input arguments
  if (args_client_init(&client_config, argc, args) != 0) {
    args_client_free(&client_config);
    return EXIT_FAILURE;
  }

  // Thread pool stuffses
  if (tpool_queue_init(&queue) != 0) {
    args_client_free(&client_config);
    tpool_queue_free(&queue);
    return EXIT_FAILURE;
  }

  // Threads: initialize
  struct tpool tp;
  tpool_init(&tp, 6);

  // Threads: spawn thread that will create buffers of words to match against hash
  // read from files with wbuffer_t
  struct wbuffer wb;
  wbuffer_init(&wb, client_config.dictionary);
  tpool_provider_create(&tp, &thread_words_from_dictionary_provider, &wb);

  // when wbuffer (dictionary files) are done, we use this wperm_t (generate words)
  // to guess further
  struct wpermutation wperm;

  // Threads: spawn threads that will consume buffers of words to match against hash
  for (int i = 0; i < tp.num_workers; i++) {
    pthread_create(tp.workers + i, NULL, &thread_encrypt_worker_local, NULL);
  }

  // Read responses from threads
  int running_workers = tp.num_workers;
  struct tpool_signal tsignal;
  while (1) {
    tpool_queue_read(queue.signal_out, &tsignal);
    if (SIGNAL_WORKLOAD_COMPLETE == tsignal.flag) {
      struct tpool_work* twork = (struct tpool_work*) tsignal.arg;
      if (twork->pass != NULL) {
        // tpool_provider_close(&tp, &thread_words_from_dictionary_provider);
        // tpool_provider_close(&tp, &thread_words_from_permutation_provider);
      }
      // printf("FREE %p\n", twork);
      free(twork->buffer);
      free(twork);
      continue;
    }
    if (SIGNAL_DICTIONARY_COMPLETE == tsignal.flag) {
      // printf("Dictionary (%s) finished.\n", client_config.dictionary);
      // stop the dictionary thread
      if (tpool_provider_close(&tp, &thread_words_from_dictionary_provider)) {
        // run word permutator (word generator)
        // printf("start perm\n");
        wperm_init(&wperm, 1);
        wperm_update(&wperm, 0);
        tpool_provider_create(&tp, &thread_words_from_permutation_provider, &wperm);
      }
      continue;
    } 
    if (SIGNAL_PERMUTATION_COMPLETE == tsignal.flag) {
      printf("Permutatuion (%d) finished.\n", wperm.word_size);
      int next = wperm.word_size + 1;
      if (tpool_provider_close(&tp, &thread_words_from_permutation_provider) && next <= client_config.length) {
        // printf("restart! %d \n", next);
        wperm_init(&wperm, next);
        wperm_update(&wperm, 0);
        tpool_provider_create(&tp, &thread_words_from_permutation_provider, &wperm);
      } else {
        tpool_provider_create(&tp, &thread_issue_close_signal, &tp.num_workers);
      }
      continue;
    }
    if (SIGNAL_WORKER_EXIT == tsignal.flag) {
      running_workers--;
      if (running_workers == 0) break;
      continue;
    }
  }

  // Waiting for worker threads to completely finish
  // printf("waiting..\n");
  for (int i = 0; i < tp.num_workers; i++) {
    pthread_join(*(tp.workers + i), NULL);
  }

  // Cleanup
  args_client_free(&client_config);
  tpool_queue_free(&queue);
  tpool_free(&tp);
  return EXIT_SUCCESS;
}