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
*/

int wdictionary_init(struct wdictionary *wdict, char* path, long* size) {
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

int wdictionary_fill(struct wdictionary* wdict, char* buffer, int cap) {
  static int length = 0;
  int offset = 0;
  if (length) fill(wdict->buffer, buffer, &offset, length, cap);
  do { length = wdictionary_read(wdict);
  } while (fill(wdict->buffer, buffer, &offset, length, cap));
  return offset;
}

void wdictionary_free(struct wdictionary* wdict) {
  fclose(wdict->fd);
  if (wdict->buffer != wdict->stack)
    free(wdict->buffer);
}