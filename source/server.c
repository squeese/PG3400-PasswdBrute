#include "args.h"
#include "tpool.h"
#include "wdictionary.h"
#include "wsolver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

struct args_client_config client_config;

static void thread_worker_cleanup(void* arg) {
  struct tpool_message msg = { 0 };
  tpool_send(((struct tpool*)arg)->queue_control, &msg, TPOOL_THREAD_EXIT, NULL, 1);
}

static int thread_worker(struct tpool* tp, struct tpool_message* msg, pthread_mutex_t* lock) {
  tpool_send(tp->queue_control, msg, TPOOL_THREAD_LOAD, NULL, 3);
  pthread_cleanup_push(thread_worker_cleanup, tp);
  while (tpool_read(tp->queue_threads, msg)) {
    if (TPOOL_WSOLVER_WORK == msg->flag) {
      if(wsolver_thread_worker(tp, msg, lock))
        continue;
    }
    if (TPOOL_WDICTIONARY_WORK == msg->flag) {
      wdictionary_thread_worker(tp, msg, lock);
      tpool_send(tp->queue_control, msg, TPOOL_WDICTIONARY_WORK, NULL, 1);
      continue;
    }
    if (TPOOL_WCOMBINATIONS_WORK == msg->flag) {
      continue;
    }
  }
  pthread_cleanup_pop(1);
  pthread_exit(0);
}

static void* thread_closer(void* arg) {
  tpool_cancel((struct tpool_context*) arg);
  pthread_exit(0);
}

int main(int argc, char** args) {
  // Create config with input arguments
  if (args_client_init(&client_config, argc, args) != 0) {
    args_client_free(&client_config);
    return EXIT_FAILURE;
  }

  // Print current configurations
  printf("> HASH    : %s\n", client_config.hash);
  printf("> INPUTS  : length:%d, %s\n", client_config.input_length, client_config.input_buffer);
  printf("> LENGTH  : start:%d, end:%d\n", client_config.word_length_min, client_config.word_length_max);
  printf("> THREAD  : count:%d, size:%d\n", client_config.thread_count, client_config.thread_buffer_size);

  // Thread pool stuffses
  struct tpool tp;
  if (tpool_init(&tp) != 0) {
    args_client_free(&client_config);
    return EXIT_FAILURE;
  }

  // Spawn thread workers
  struct tpool_context** threads = malloc(client_config.thread_count * sizeof(struct tpool_context*));
  for (int i = 0; i < client_config.thread_count; i++) {
    *(threads + i) = tpool_run(&tp, thread_worker);
    pthread_detach(threads[i]->thread);
  }

  // Queue the initial message to get things started
  struct tpool_message msg = { 0 };
  if (client_config.dictionary_count > 0) {
    tpool_send(tp.queue_threads, &msg, TPOOL_WDICTIONARY_WORK, *client_config.dictionary_paths, 1);
  } else {
    tpool_send(tp.queue_threads, &msg, TPOOL_WCOMBINATIONS_WORK, &client_config.word_length_min, 1);
  }

  // Spawn thread controller
  char* password = NULL;
  int threads_active = 0;
  while (tpool_read(tp.queue_control, &msg)) {
    if (TPOOL_THREAD_LOAD == msg.flag) {
      printf("++\n");
      threads_active++;
      continue;
    }
    if (TPOOL_THREAD_EXIT == msg.flag) {
      printf("--\n");
      if (--threads_active == 0) break;
      continue;
    }
    if (TPOOL_WSOLVER_WORK == msg.flag) {
      struct wsolver_work* work = msg.arg;
      if (work->pass != NULL) {
        int len = strlen(work->pass);
        password = malloc((len + 1) * sizeof(char));
        memcpy(password, work->pass, len);
        *(password + len) = 0;
        printf("Success: %s\n", password);
        // tpool_provider_close(&tp, &thread_words_from_dictionary_provider);
        // tpool_provider_close(&tp, &thread_words_from_permutation_provider);
        for (int i = 0; i < client_config.thread_count; i++) {
          pthread_t t;
          pthread_create(&t, NULL, thread_closer, (threads + i));
          pthread_detach(t);
        }
        // break;
      }
      // progress_update(&prog, twork->length);
      free(work->buffer);
      free(work);
      continue;
    }
    if (TPOOL_WDICTIONARY_WORK == msg.flag) {
      printf("dictionary done\n");
      continue;
    }
  }

  // Success
  if (password != NULL) {
    printf("> Password: %s\n", password);
    free(password);
  } else {
    printf("> No match\n");
  }

  // Cleanup
  free(threads);
  args_client_free(&client_config);
  tpool_free(&tp);
  return EXIT_SUCCESS;
}