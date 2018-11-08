#include "args.h"
#include "client_thandlers.h"
#include "tpool.h"
#include "wbuffer.h"
#include "wpermutation.h"
#include "progress.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

struct args_client_config client_config;
struct tqueue_queue queue;

int main(int argc, char** args) {
  // Create config with input arguments
  if (args_client_init(&client_config, argc, args) != 0) {
    args_client_free(&client_config);
    return EXIT_FAILURE;
  }

  printf("> HASH    : %s\n", client_config.hash);
  printf("> INPUTS  : length:%d, %s\n", client_config.input_length, client_config.input_buffer);
  printf("> LENGTH  : start:%d, end:%d\n", client_config.word_length_min, client_config.word_length_max);
  printf("> THREAD  : count:%d, size:%d\n", client_config.thread_count, client_config.thread_buffer_size);

  // Thread pool stuffses
  if (tqueue_queue_init(&queue) != 0) {
    args_client_free(&client_config);
    tqueue_queue_free(&queue);
    return EXIT_FAILURE;
  }

  int dictionary_index = 0;
  int combinator_index = client_config.word_length_min;
  int running_workers = 0;
  char* password = NULL;
  struct tpool tp;
  struct progress prog;
  struct tqueue_signal tsignal;
  struct wbuffer wb;
  struct wpermutation wperm;
  memset(&tsignal, 0, sizeof(struct tqueue_signal));
  tqueue_init(&tp, client_config.thread_count);
  for (int i = 0; i < tp.num_workers; i++)
    pthread_create(tp.workers + i, NULL, &thread_worker_local_encrypt, NULL);
  running_workers = client_config.thread_count;
  tsignal.flag = SIGNAL_PROVIDER_START;
  tqueue_queue_send(queue.signal_out, &tsignal, 2);

  // Read signales from threads
  while (tqueue_queue_read(queue.signal_out, &tsignal)) {

    if (SIGNAL_WORKLOAD_COMPLETED == tsignal.flag) {
      struct tqueue_work* twork = (struct tqueue_work*) tsignal.arg;
      if (twork->pass != NULL) {
        int len = strlen(twork->pass);
        password = malloc((len + 1) * sizeof(char));
        memcpy(password, twork->pass, len);
        *(password + len) = 0;
        tqueue_provider_close(&tp, &thread_words_from_dictionary_provider);
        tqueue_provider_close(&tp, &thread_words_from_permutation_provider);
      }
      progress_update(&prog, twork->length);
      free(twork->buffer);
      free(twork);
      continue;
    }

    if (SIGNAL_DICTIONARY_COMPLETED == tsignal.flag) {
      dictionary_index++;
      progress_finish(&prog);
      // stop the dictionary thread
      if (tqueue_provider_close(&tp, &thread_words_from_dictionary_provider) == 0) {
        // the provider was closed via pthread, that means we found a password
        tqueue_provider_create(&tp, &thread_issue_close_signal, &client_config.thread_count);
        continue;
      }
    }

    if (SIGNAL_PERMUTATION_COMPLETED == tsignal.flag) {
      combinator_index++;
      progress_finish(&prog);
      if (tqueue_provider_close(&tp, &thread_words_from_permutation_provider) == 0) {
        // the provider was closed via phtread, that means we found a password
        tqueue_provider_create(&tp, &thread_issue_close_signal, &client_config.thread_count);
        continue;
      }
    }

    if ((SIGNAL_PROVIDER_START | SIGNAL_DICTIONARY_COMPLETED | SIGNAL_PERMUTATION_COMPLETED) & tsignal.flag) {
      if (dictionary_index < client_config.dictionary_count) {
        progress_init(&prog);
        // Spawn word dictionary provider
        char* path = *(client_config.dictionary_paths + dictionary_index);
        if (wbuffer_init(&wb, path, &prog.max) != 0) {
          tqueue_provider_create(&tp, &thread_issue_close_signal, &client_config.thread_count);
        } else {
          progress_title(&prog, snprintf(prog.title, 64, "Dictionary: %s", path));
          tqueue_provider_create(&tp, &thread_words_from_dictionary_provider, &wb);
        }
      } else if (combinator_index <= client_config.word_length_max) {
        progress_init(&prog);
        wperm_init(&wperm, combinator_index, client_config.input_buffer, client_config.input_length, &prog.max);
        progress_title(&prog, snprintf(prog.title, 64, "Word Size %d, Search Area %d, Solutions %ld", combinator_index, client_config.input_length, wperm.solutions));
        tqueue_provider_create(&tp, &thread_words_from_permutation_provider, &wperm);
      } else {
        tqueue_provider_create(&tp, &thread_issue_close_signal, &client_config.thread_count);
      }
      continue;
    }

    if (SIGNAL_WORKER_EXIT == tsignal.flag) {
      if (--running_workers == 0) break;
    }
  }
  printf("\n\n");

  // Waiting for worker threads to completely finish
  for (int i = 0; i < tp.num_workers; i++) {
    pthread_join(*(tp.workers + i), NULL);
  }

  // Reportskies
  if (password != NULL) {
    printf("> PASSWORD: %s\n", password);
    free(password);
  } else {
    printf("> PASSWORD not found =(\n");
  }

  // Cleanup
  args_client_free(&client_config);
  tqueue_queue_free(&queue);
  tqueue_free(&tp);
  return EXIT_SUCCESS;
}
/*


     main thread       theads                            mainthread
                       threads
    provider

    threads
    threads
    thread



*/