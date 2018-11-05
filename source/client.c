#define _GNU_SOURCE 1
#include "args.h"
#include "wbuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <wait.h>
#include <pthread.h>
#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <crypt.h>

enum INSTRUCTION {
  THREAD_INSTRUCTION_CLOSE = 0,
  THREAD_INSTRUCTION_WORK = 1,
  THREAD_INSTRUCTION_DICTIONARY_COMPLETE = 2,
};

struct thread_instruction {
  enum INSTRUCTION flag;
  void* arg;
};

struct thread_work {
  char* salt;
  char* hash;
  int size_buffer;
  char* word_buffer;
  char* pass;
};

struct thread_queue {
  struct mq_attr instructions_attr;
  struct mq_attr responses_attr;
  mqd_t instructions;
  mqd_t responses;
};

static int NUM_THREADS = 12;
static struct args_client_config client_config;

struct word_dictionary {
  FILE* file;
  char* line;
};

int word_dictionary_init(struct word_dictionary* wdict, char* path) {
  wdict->file = fopen(path, "r");
  wdict->line = NULL;
  if (wdict->file == NULL) {
    printf("Unable to open dictionary file at given path: %s\nError: (%d) %s\n", path, errno, strerror(errno));
    return errno;
  }
  return 0;
}

void word_dictionary_free(struct word_dictionary* wdict) {
  fclose(wdict->file);
  if (wdict->line != NULL) {
    free(wdict->line);
    wdict->line = NULL;
  }
}

int word_push(char* buffer, char* line, int* offset, int len, int cap) {
  if (len > 0 && (*offset + len) < cap) {
    memcpy(buffer + *offset, line, len - 1);
    *(buffer + (*offset + len - 1)) = 0;
    *offset += len;
    return 1;
  }
  return 0;
}

int word_dictionary_next(struct word_dictionary* wdict, struct thread_work* twork, int size_buffer) {
  static size_t len_bytes = 0;
  static ssize_t len_chars = 0;
  twork->word_buffer = malloc(size_buffer * sizeof(char));
  // printf("MALL: %p (word_buffer) word_dict_next\n", twork->word_buffer);
  int offset = 0;
  if (wdict->line == NULL) {
    wdict->line = malloc(32 * sizeof(char));
  } else {
    if (!word_push(twork->word_buffer, wdict->line, &offset, len_chars, size_buffer)) return 0;
  }
  do {
    len_chars = getline(&wdict->line, &len_bytes, wdict->file);
  } while(word_push(twork->word_buffer, wdict->line, &offset, len_chars, size_buffer));
  twork->size_buffer = offset;
  return offset;
}

static void* worker_thread_handler(void* arg) {
  static int ID = 0;
  int id = ID++;
  struct thread_queue* tqueue = (struct thread_queue*) arg;
  struct thread_instruction tinstruction = {};
  char* encoded;
  struct crypt_data crypt;
	crypt.initialized = 0;
  while (1) {
    mq_receive(tqueue->instructions, (char *) &tinstruction, sizeof(struct thread_instruction), NULL);
    if (tinstruction.flag == THREAD_INSTRUCTION_CLOSE) {
      printf("(%d) exit\n", id);
      mq_send(tqueue->responses, (const char*) &tinstruction, sizeof(struct thread_instruction), 1);
      break;
    }
    if (tinstruction.flag == THREAD_INSTRUCTION_WORK) {
      struct thread_work* twork = (struct thread_work*) tinstruction.arg;
      printf("(%d) instruction %s\n", id, twork->word_buffer);
      for (int i = 0; i < twork->size_buffer; ) {
        encoded = crypt_r(twork->word_buffer + i, twork->salt, &crypt);
        if (strncmp(encoded, twork->hash, 34) == 0) {
          printf("MATCH %s\n", twork->word_buffer + i);
          twork->pass = twork->word_buffer + i;
          break;
        }
        i += strlen(twork->word_buffer + i) + 1;
      }
      printf("(%d) send response\n", id);
      mq_send(tqueue->responses, (const char*) &tinstruction, sizeof(struct thread_instruction), 2);
    }
    usleep(10000);
  }
  pthread_exit(0);
}

void cleanup(void* arg) {
  struct wbuffer* wb = arg;
  wbuffer_free(wb);
  printf("cleanup\n");
}

void cleanup2(void* arg) {
  struct thread_queue* queue = arg;
  struct thread_instruction tinstruction = { THREAD_INSTRUCTION_DICTIONARY_COMPLETE, NULL };
  mq_send(queue->responses, (const char*) &tinstruction, sizeof(struct thread_instruction), 1);
  printf("cleanup2\n");
}

void cleanup3(void* arg) {
  char** buffer = arg;
  free(*buffer);
  printf("cleanup3\n");
}

static void* worker_thread_dictionary_provider(void* arg) {
  struct thread_queue* queue = arg;
  struct thread_instruction tinstruction = {};
  struct wbuffer wb;
  char* buffer;
  wbuffer_init(&wb, client_config.dictionary);
  pthread_cleanup_push(&cleanup, &wb);
  pthread_cleanup_push(&cleanup2, queue);
  pthread_cleanup_push(&cleanup3, &buffer);
  while (1) {
    buffer = malloc(sizeof(char) * 32);
    // if (word_dictionary_next(&wdict, twork, 1024)) {
    if (wbuffer_fill(&wb, buffer, 32)) {
      struct thread_work* twork = malloc(sizeof(struct thread_work));
      twork->salt = client_config.salt;
      twork->hash = client_config.hash;
      twork->word_buffer = buffer;
      twork->pass = NULL;
      tinstruction.arg = twork;
      tinstruction.flag = THREAD_INSTRUCTION_WORK;
      // printf("-> instruction %s\n", twork->word_buffer);
      mq_send(queue->instructions, (const char*) &tinstruction, sizeof(struct thread_instruction), 2);
    } else {
      // free(buffer);
      // tinstruction.arg = NULL;
      // tinstruction.flag = THREAD_INSTRUCTION_DICTIONARY_COMPLETE;
      // mq_send(queue->responses, (const char*) &tinstruction, sizeof(struct thread_instruction), 1);
      break;
    }
    usleep(1000);
  }
  // word_dictionary_free(&wbuffer);
  // wbuffer_free(&wb);
  pthread_cleanup_pop(1);
  pthread_cleanup_pop(1);
  pthread_cleanup_pop(1);
  printf("EXIT\n");
  pthread_exit(0);
}

static void* worker_thread_closer(void* arg) {
  struct thread_queue* queue = arg;
  struct thread_instruction tinstruction = { THREAD_INSTRUCTION_CLOSE };
  for (int i = 0; i < NUM_THREADS; i++) {
    mq_send(queue->instructions, (const char*) &tinstruction, sizeof(struct thread_instruction), 3);
    // printf("EXIT FOR %d !!\n", i);
  }
  pthread_exit(0);
}

int main(int argc, char** args) {
  // Create config with input arguments
  if (args_client_init(&client_config, argc, args) != 0) {
    return EXIT_FAILURE;
  }

  // printf("path: %s\n", client_config.dictionary);

  // Create message queues, which will be used to communicate between threads
  struct thread_queue queue;
  queue.instructions_attr.mq_maxmsg = 8;
  queue.instructions_attr.mq_msgsize = sizeof(struct thread_instruction);
  queue.responses_attr.mq_maxmsg = 8; 
  queue.responses_attr.mq_msgsize = sizeof(struct thread_instruction);
  mq_unlink("/passcrkwrkcl");
  mq_unlink("/passcrkrescl");
  queue.instructions = mq_open("/passcrkwrkcl", O_CREAT | O_RDWR, 0666, &queue.instructions_attr); //  | O_NONBLOCK
  queue.responses = mq_open("/passcrkrescl", O_CREAT | O_RDWR, 0666, &queue.responses_attr); //  | O_NONBLOCK
  if (queue.instructions == -1 || queue.responses == -1) {
    fprintf(stderr, "Error opening messaging queue. errno(%d): %s\n", errno, strerror(errno));
    args_client_free(&client_config);
    exit(EXIT_FAILURE);
  }

  // Worker threads
  // printf("> start threads.\n");
  pthread_t* worker_threads = malloc((NUM_THREADS + 1) * sizeof(pthread_t));
  // printf("MALL: %p (worker_threads) main\n", worker_threads);
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_create(worker_threads + i, NULL, &worker_thread_handler, &queue);
  }

  pthread_create(worker_threads + NUM_THREADS, NULL, &worker_thread_dictionary_provider, &queue);
  pthread_detach(*(worker_threads + NUM_THREADS));

  // Read responses from threads
  struct thread_instruction tinstruction;
  int threads_open = NUM_THREADS;
  char* password = NULL;
  while (1) {
    mq_receive(queue.responses, (char *) &tinstruction, sizeof(struct thread_instruction), NULL);
    if (tinstruction.flag == THREAD_INSTRUCTION_WORK) {
      struct thread_work* twork = (struct thread_work*) tinstruction.arg;
      printf(".");
      fflush(stdout);
      if (twork->pass != NULL) {
        password = twork->pass;
        printf("Success: %s\n", twork->pass);
        free(twork->word_buffer);
        free(twork);
        pthread_cancel(*(worker_threads + NUM_THREADS));
      }
      free(twork->word_buffer);
      free(twork);
    } else if (tinstruction.flag == THREAD_INSTRUCTION_DICTIONARY_COMPLETE) {
      printf("dict complete\n");
      if (password == NULL) {
        // close threads
        printf("close threads\n");
        pthread_t worker_close;
        pthread_create(&worker_close, NULL, &worker_thread_closer, &queue);
        pthread_detach(worker_close);
      }
    } else if (tinstruction.flag == THREAD_INSTRUCTION_CLOSE) {
      if (--threads_open == 0) break;
    }
  }

  // Wait for worker threads
  printf("waiting..\n");
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(*(worker_threads + i), NULL);
  }
  free(worker_threads);
  // Cancel job creator thread if its running
  // pthread_cancel(*(worker_threads + NUM_THREADS));

  args_client_free(&client_config);

  mq_close(queue.instructions);
  mq_close(queue.responses);
  mq_unlink("/passcrkwrkcl");
  mq_unlink("/passcrkrescl");
  return EXIT_SUCCESS;
} 
