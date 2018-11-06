#define _GNU_SOURCE 1
#include "client_thandlers.h"
#include "wbuffer.h"
#include "wpermutation.h"
#include <stdlib.h>
#include <string.h>
#include <crypt.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>

void* thread_worker_local_encrypt(void* arg) {
  struct tpool_signal tsignal;
  memset(&tsignal, 0, sizeof(struct tpool_signal));
  struct crypt_data crypt;
	crypt.initialized = 0;
  char* encoded;
  while (tpool_queue_read(queue.signal_in, &tsignal)) {
    if (SIGNAL_WORKLOAD_START == tsignal.flag) {
      struct tpool_work* twork = tsignal.arg;
      for (int i = 0; i < twork->length; ) {
        encoded = crypt_r(twork->buffer + i, twork->salt, &crypt);
        if (strncmp(encoded, twork->hash, 34) == 0) {
          twork->pass = twork->buffer + i;
          break;
        }
        i += strlen(twork->buffer + i) + 1;
      }
      tsignal.flag = SIGNAL_WORKLOAD_COMPLETED;
      tsignal.arg = twork;
      tpool_queue_send(queue.signal_out, &tsignal, 2);
      continue;
    }
    if (SIGNAL_WORKER_EXIT == tsignal.flag) {
      // printf("thread(%d) exit.\n", id);
      tsignal.flag = SIGNAL_WORKER_EXIT;
      tsignal.arg = NULL;
      tpool_queue_send(queue.signal_out, &tsignal, 1);
      break;
    }
  }
  pthread_exit(0);
}

static void dictionary_provider_cleanup(void* arg) {
  // printf("dictionary_provider_cleanup\n");
  struct thread_cleanup_t* tct = arg;
  wbuffer_free(tct->arg);
  if (*tct->buffer != NULL) {
    free(*tct->buffer);
  }
  struct tpool_signal tsignal;
  memset(&tsignal, 0, sizeof(struct tpool_signal));
  tsignal.flag = SIGNAL_DICTIONARY_COMPLETED;
  tpool_queue_send(queue.signal_out, &tsignal, 1);
}

void* thread_words_from_dictionary_provider(void* arg) {
  struct tpool_signal tsignal;
  memset(&tsignal, 0, sizeof(struct tpool_signal));
  char* buffer = NULL;
  struct wbuffer* wbuf = arg;
  struct thread_cleanup_t tct = { &buffer, wbuf };
  pthread_cleanup_push(&dictionary_provider_cleanup, &tct);
  while (1) {
    buffer = malloc(client_config.thread_buffer_size * sizeof(char));
    int length;
    if ((length = wbuffer_fill(wbuf, buffer, client_config.thread_buffer_size)) > 0) {
      struct tpool_work* twork = malloc(sizeof(struct tpool_work));
      // printf("MALLOC %p\n", twork);
      twork->salt = client_config.salt;
      twork->hash = client_config.hash;
      twork->buffer = buffer;
      buffer = NULL;
      twork->length = length;
      twork->pass = NULL;
      tsignal.flag = SIGNAL_WORKLOAD_START;
      tsignal.arg = twork;
      tpool_queue_send(queue.signal_in, &tsignal, 1);
      // progress_add_max(&prog, length);
      continue;
    }
    break;
  }
  pthread_cleanup_pop(1);
  pthread_exit(0);
}

static void permutation_provider_cleanup(void* arg) {
  // printf("permutation_provider_cleanup\n");
  struct thread_cleanup_t* tct = arg;
  wperm_free(tct->arg);
  if (*tct->buffer != NULL) {
    // printf("FREE BUFFER %p\n", *tct->buffer);
    free(*tct->buffer);
  }
  struct tpool_signal tsignal;
  memset(&tsignal, 0, sizeof(struct tpool_signal));
  tsignal.flag = SIGNAL_PERMUTATION_COMPLETED;
  tpool_queue_send(queue.signal_out, &tsignal, 1);
}

void* thread_words_from_permutation_provider(void* arg) {
  struct tpool_signal tsignal;
  memset(&tsignal, 0, sizeof(struct tpool_signal));
  char* buffer = NULL;
  struct wpermutation* wperm = arg;
  struct thread_cleanup_t tct = { &buffer, wperm };
  pthread_cleanup_push(&permutation_provider_cleanup, &tct);
  while (1) {
    buffer = malloc(client_config.thread_buffer_size * sizeof(char));
    int count = client_config.thread_buffer_size / (sizeof(char) * (wperm->word_size + 1));
    int length;
    if ((length = wperm_generate(wperm, buffer, count)) > 0) {
      struct tpool_work* twork = malloc(sizeof(struct tpool_work));
      twork->salt = client_config.salt;
      twork->hash = client_config.hash;
      twork->buffer = buffer;
      buffer = NULL;
      twork->length = length;
      twork->pass = NULL;
      tsignal.flag = SIGNAL_WORKLOAD_START;
      tsignal.arg = twork;
      tpool_queue_send(queue.signal_in, &tsignal, 1);
      continue;
    }
    break;
  }

  pthread_cleanup_pop(1);
  pthread_exit(0);
}

void* thread_issue_close_signal(void* arg) {
  struct tpool_signal tsignal;
  memset(&tsignal, 0, sizeof(struct tpool_signal));
  tsignal.flag = SIGNAL_WORKER_EXIT;
  for (int i = *(int*) arg; i > 0; i--) {
    tpool_queue_send(queue.signal_in, &tsignal, 1);
  }
  pthread_exit(0);
}