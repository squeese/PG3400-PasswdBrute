#include "wbuffer.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int wbuffer_init(struct wbuffer *wb, char* path, long* size) {
  wb->word = wb->stack;
  wb->index = 0;
  wb->size = WBUFFER_INITIAL_SIZE;
  wb->fd = fopen(path, "r");
  if (wb->fd == NULL) {
    printf("Unable to open dictionary file at given path: %s\nError: (%d) %s\n", path, errno, strerror(errno));
    return errno;
  }
  if (size != NULL) {
    fseek(wb->fd, 0, SEEK_END);
    *size = ftell(wb->fd);
    fseek(wb->fd, 0, SEEK_SET);
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

int wbuffer_fill(struct wbuffer* wb, char* buffer, int cap) {
  static int length = 0;
  int offset = 0;
  // for (int i = 0; i < cap; i++) *(buffer + i) = 0;
  if (length) fill(wb->word, buffer, &offset, length, cap);
  do {
    length = wbuffer_read(wb);
  } while (fill(wb->word, buffer, &offset, length, cap));
  return offset;
}

int wbuffer_read(struct wbuffer* wb) {
  int cursor;
  wbuffer_reset(wb);
  while ((cursor = getc(wb->fd)) != EOF) {
    if (cursor == '\n' || cursor == '\r') {
      do { cursor = getc(wb->fd);
      } while(cursor == '\n' || cursor == '\r');
      ungetc(cursor, wb->fd);
      break;
    }
    wbuffer_write(wb, cursor);
  }
  return wb->index;
}

void wbuffer_write(struct wbuffer* wb, char c) {
  if (wb->index == wb->size) {
    int size = wb->size * 2;
    if (wb->word == wb->stack) {
      wb->word = malloc(size * sizeof(char));
      memcpy(wb->word, wb->stack, WBUFFER_INITIAL_SIZE);
    } else {
      wb->word = realloc(wb->word, size * sizeof(char));
      wb->size = size;
    }
  }
  *(wb->word + wb->index++) = c;
}

void wbuffer_reset(struct wbuffer* wb) {
  wb->index = 0;
}

void wbuffer_free(struct wbuffer* wb) {
  fclose(wb->fd);
  if (wb->word != wb->stack) {
    free(wb->word);
  }
}