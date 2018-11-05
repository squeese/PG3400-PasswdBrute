#define _GNU_SOURCE 1
#include "client_thandlers.h"
#include "wbuffer.h"
#include "wpermutation.h"
#include "tpool.h"
#include <stdlib.h>
#include <string.h>
#include <crypt.h>

void* thread_encrypt_worker_local(void* arg) {
  static int ID = 0;
  int id = ID++;
  struct tpool_signal tsignal = { 0, 0 };
  char* encoded;
  struct crypt_data crypt;
	crypt.initialized = 0;
  while (1) {
    tpool_queue_read(queue.signal_in, &tsignal);
    if (SIGNAL_WORKLOAD_START == tsignal.flag) {
      struct tpool_work* twork = tsignal.arg;
      // printf("thread(%d) instruction %s\n", id, twork->buffer);
      for (int i = 0; i < twork->length; ) {
        // printf("trhead(%d) trying `%s`, len: %ld\n", id, twork->buffer + i, strlen(twork->buffer + i));
        encoded = crypt_r(twork->buffer + i, twork->salt, &crypt);
        if (strncmp(encoded, twork->hash, 34) == 0) {
          printf("------------------------------------ MATCH %s => %s\n", twork->buffer + i, twork->hash);
          twork->pass = twork->buffer + i;
          break;
        }
        i += strlen(twork->buffer + i) + 1;
      }
      tsignal.flag = SIGNAL_WORKLOAD_COMPLETE;
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
  struct tpool_signal tsignal = { SIGNAL_DICTIONARY_COMPLETE, NULL };
  tpool_queue_send(queue.signal_out, &tsignal, 1);
}

void* thread_words_from_dictionary_provider(void* arg) {
  struct tpool_signal tsignal;
  char* buffer = NULL;
  struct wbuffer* wbuf = arg;
  struct thread_cleanup_t tct = { &buffer, wbuf };
  pthread_cleanup_push(&dictionary_provider_cleanup, &tct);
  while (1) {
    break;
    buffer = malloc(sizeof(char) * 512);
    int length;
    if ((length = wbuffer_fill(wbuf, buffer, 512)) > 0) {
      struct tpool_work* twork = malloc(sizeof(struct tpool_work));
      // printf("MALLOC %p\n", twork);
      twork->salt = client_config.salt;
      twork->hash = client_config.hash;
      twork->buffer = buffer;
      twork->length = length;
      twork->pass = NULL;
      tsignal.flag = SIGNAL_WORKLOAD_START;
      tsignal.arg = twork;
      tpool_queue_send(queue.signal_in, &tsignal, 1);
      buffer = NULL;
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
    free(*tct->buffer);
  }
  struct tpool_signal tsignal = { SIGNAL_PERMUTATION_COMPLETE, NULL };
  tpool_queue_send(queue.signal_out, &tsignal, 1);
}

void* thread_words_from_permutation_provider(void* arg) {
  struct tpool_signal tsignal;
  char* buffer = NULL;
  struct wpermutation* wperm = arg;
  struct thread_cleanup_t tct = { &buffer, wperm };
  pthread_cleanup_push(&permutation_provider_cleanup, &tct);
  while (1) {
    buffer = malloc(wperm_buffer_size(wperm, 128));
    int length;
    if ((length = wperm_generate(wperm, buffer, 128)) > 0) {
      struct tpool_work* twork = malloc(sizeof(struct tpool_work));
      twork->salt = client_config.salt;
      twork->hash = client_config.hash;
      twork->buffer = buffer;
      twork->length = length;
      twork->pass = NULL;
      tsignal.flag = SIGNAL_WORKLOAD_START;
      tsignal.arg = twork;
      tpool_queue_send(queue.signal_in, &tsignal, 1);
      buffer = NULL;
      continue;
    }
    break;
  }

  pthread_cleanup_pop(1);
  pthread_exit(0);
}

void* thread_issue_close_signal(void* arg) {
  struct tpool_signal tsignal = { SIGNAL_WORKER_EXIT, NULL };
  for (int i = *(int*) arg; i > 0; i--) {
    tpool_queue_send(queue.signal_in, &tsignal, 1);
  }
  pthread_exit(0);
}