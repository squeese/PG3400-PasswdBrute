#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
// #include <srand.h>

static void* queue_run_thread(void* arg) {
  struct queue* q = (struct queue*) arg;
  struct job* j;
  struct crypt_data crypt;
  crypt.initialized = 0;
  char* enc;
  while ((j = queue_pop_job(q)) != NULL) {
    for (int i = j->offset; i >= 0; i--) {
      enc = crypt_r(*(j->words - i), q->salt, &crypt);
      // printf("%s, %s %s %s\n", q->salt, enc, q->hash, *(j->words - i));
      if (strncmp(enc, q->hash, 34) == 0) {
        printf("SUCCESS (%d) %s %s\n", i, *(j->words - i), enc);
        queue_free_job(j);
        // queue_stop(q);
        pthread_exit(0);
        return NULL;
      }
    }
    queue_free_job(j);
  }
  pthread_exit(0);
}

int queue_init(struct queue* q, struct dictionary* d, char* hash) {
  if (pthread_mutex_init(&q->lock, NULL) != 0) {
    printf("Unable to initate mutex lock.\nError: (%d) %s\n", errno, strerror(errno));
  }
  q->dict = d;
  q->jobs = calloc(QUEUE_JOBS, sizeof(struct job**));
  q->threads = malloc(QUEUE_THREADS * sizeof(pthread_t));
  q->job_cursor = 0;
  q->dict_cursor = 0;
  memcpy(&q->salt, hash, 12);
  q->salt[12] = '\0';
  q->hash = hash;
  return 0;
}

void queue_start(struct queue* q) {
  // run threads
  for (int i = 0; i < QUEUE_THREADS; i++) {
    printf("<thread:start:%d>\n", i);
    pthread_create(&q->threads[i], NULL, queue_run_thread, q);
  }
  // wait for threads to finish
  for (int i = 0; i < QUEUE_THREADS; i++) {
    pthread_join(q->threads[i], NULL);
    printf("<thread:done:%d>\n", i);
  }
}

int queue_push_job(struct queue* q, unsigned int index) {
  if (q->dict_cursor >= q->dict->count) {
    return EXIT_FAILURE;
  }
  unsigned word_start = q->dict_cursor;
  unsigned word_end = word_start + QUEUE_STRIDE;
  if (word_end >= q->dict->count) {
    // asdfasdfaslkdfj
    word_end = q->dict->count - 1;
    q->dict_cursor++;
  }
  struct job* j = malloc(sizeof(struct job));
  j->num = index;
  j->offset = word_end - word_start;
  j->words = &q->dict->entries[word_end];
  q->jobs[index] = j;
  q->dict_cursor += j->offset;
  // printf("queue_push_job < index:%d cursor:%d offset:%d count:%d\n", index, q->dict_cursor, j->offset, q->dict->count);
  return EXIT_SUCCESS;
}

struct job* queue_pop_job(struct queue* q) {
  pthread_mutex_lock(&q->lock);
  struct job* j = q->jobs[q->job_cursor];
  if (j == NULL) {
    if (q->dict_cursor >= q->dict->count) {
      pthread_mutex_unlock(&q->lock);
      return NULL;
    }
    for (unsigned int i = 0; (i < QUEUE_JOBS) && (queue_push_job(q, i) == EXIT_SUCCESS); i++);
    q->job_cursor = 0;
    pthread_mutex_unlock(&q->lock);
    return queue_pop_job(q);
  }
  q->jobs[q->job_cursor] = NULL;
  q->job_cursor++;
  pthread_mutex_unlock(&q->lock);
  return j;
}

void queue_free_job(struct job* j) {
  free(j);
}

void queue_free(struct queue* q) {
  pthread_mutex_destroy(&q->lock);
  dict_free(q->dict);
}
