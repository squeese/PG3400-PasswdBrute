#include "wdictionary.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*
  Purpose of wdictionary is to read words from a given file, and be
  able to fill a buffer with as many words it can fit, and all words
  separated by \0 terminator. That way you can iterate over the buffer
  very compactly, and provide the crack_r() function with a pointer to
  each words beginning.

  The only visible function from here is the wdictionary_worker. It's
  being assigned to a tpool (phtread) thread to provide other workers
  (threads) with data.

  It will create a buffer of fixed size (can be changed via terminal args)
  and put them on a messaging queue (mqueue), where other threads can pick
  them off the queue and process them.

  The buffer is allocated on the heap, so when the 'message' with the buffer
  has been sent, wdictionary_worker is no longer responsible to dealloc it,
  its the responsebility of the other threads picking the message up.
*/

static int wdictionary_init(struct wdictionary *wdict, char* path, long* size) {
  wdict->buffer = wdict->stack;
  wdict->index = 0;
  wdict->size = WDICTIONARY_INITIAL_BUFFER_SIZE;
  wdict->fd = fopen(path, "r");
  if (wdict->fd == NULL) {
    printf("Unable to open dictionary file at given path: %s\nError: (%d) %s\n", path, errno, strerror(errno));
    return errno;
  }
  if (size != NULL) {
    fseek(wdict->fd, 0, SEEK_END);
    *size = ftell(wdict->fd);
    fseek(wdict->fd, 0, SEEK_SET);
  }
  return 0;
}

static int fill(char* line, char* buffer, int* offset, int length, int cap) {
  if (length == 0 || (*offset + length) >= cap) return 0;
  memcpy(buffer + *offset, line, length + 1);
  *(buffer + (*offset + length)) = 0;
  *offset += length + 1;
  return 1;
}

static void wdictionary_write(struct wdictionary* wdict, char c) {
  if ((wdict->index + 1) == wdict->size) {
    int size = wdict->size * 2;
    if (wdict->buffer == wdict->stack) {
      wdict->buffer = malloc(size * sizeof(char));
      memcpy(wdict->buffer, wdict->stack, WDICTIONARY_INITIAL_BUFFER_SIZE);
    } else {
      wdict->buffer = realloc(wdict->buffer, size * sizeof(char));
      wdict->size = size;
    }
  }
  *(wdict->buffer + wdict->index++) = c;
}

static int wdictionary_read(struct wdictionary* wdict) {
  int cursor;
  wdict->index = 0;
  while ((cursor = getc(wdict->fd)) != EOF) {
    if (cursor == '\n' || cursor == '\r') {
      do { cursor = getc(wdict->fd);
      } while(cursor == '\n' || cursor == '\r');
      ungetc(cursor, wdict->fd);
      break;
    }
    wdictionary_write(wdict, cursor);
  }
  *(wdict->buffer + wdict->index) = 0;
  return wdict->index;
}

static int wdictionary_fill(struct wdictionary* wdict, char* buffer, int cap) {
  static int length = 0;
  int offset = 0;
  if (length) fill(wdict->buffer, buffer, &offset, length, cap);
  do { length = wdictionary_read(wdict);
  } while (fill(wdict->buffer, buffer, &offset, length, cap));
  return offset;
}

static void wdictionary_cleanup(void* arg) {
  struct wdictionary* wdict = arg;
  fclose(wdict->fd);
  if (wdict->buffer != wdict->stack)
    free(wdict->buffer);
}

int wdictionary_thread_worker(struct tpool* tp, struct tpool_message* msg, pthread_mutex_t* lock) {
  struct wdictionary wdict;
  if (wdictionary_init(&wdict, (char*)msg->arg, NULL) != 0) {
    printf("wtf \n");
  }
  pthread_cleanup_push(&wdictionary_cleanup, &wdict);
  do {
    pthread_mutex_lock(lock);
    char* buffer = malloc(client_config.thread_buffer_size * sizeof(char));
    int length;
    if ((length = wdictionary_fill(&wdict, buffer, client_config.thread_buffer_size)) > 0) {
      struct tmp_work* twork = malloc(sizeof(struct tmp_work));
      twork->buffer = buffer;
      twork->length = length;
      tpool_send(tp->queue_threads, msg, TPOOL_WSOLVER_WORK, twork, 1);
      pthread_mutex_unlock(lock);
      continue;
    }
    free(buffer);
    pthread_mutex_unlock(lock);
    break;
  } while(1);
  pthread_cleanup_pop(1);
  return 0;
}
