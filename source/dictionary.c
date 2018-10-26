#include "dictionary.h"
#include <stdio.h> // fopen
#include <stdlib.h> // malloc, calloc
#include <string.h> // memcpy, printf, strerror
#include <errno.h> // errno variable
#include <ctype.h>

/*
static char CRLF[2] = { '\r', '\n' };
static char LF[1] = { '\n' };

static void detect_delimiter(FILE* fd) {
  int cursor;
  while ((cursor = getc(fd)) != EOF) {
    if ()
  }
}

static void dict_scan(FILE* fd, void* fn) {
}
*/

// https://stackoverflow.com/questions/41946007/efficient-and-well-explained-implementation-of-a-quadtree-for-2d-collision-det

enum {
  INPUT_BUFFER_INITIAL_SIZE = 16
};

struct ibuffer {
  char stack[INPUT_BUFFER_INITIAL_SIZE];
  char* data;
  int index;
  int size;
  FILE* fd;
};

int ibuffer_init(struct ibuffer *ib, char* path) {
  ib->data = ib->stack;
  ib->index = 0;
  ib->size = INPUT_BUFFER_INITIAL_SIZE;
  ib->fd = fopen(path, "r");
  if (ib->fd == NULL) {
    printf("Unable to open dictionary file at given path: %s\nError: (%d) %s\n", path, errno, strerror(errno));
    return errno;
  }
  return 0;
}

void ibuffer_free(struct ibuffer *ib) {
  fclose(ib->fd);
  if (ib->data != ib->stack) {
    free(ib->data);
  }
}

void ibuffer_write(struct ibuffer *ib, char c) {
  if (ib->index == ib->size) {
    int size = ib->size * 2;
    if (ib->data == ib->stack) {
      ib->data = malloc(size * sizeof(char));
      memcpy(ib->data, ib->stack, INPUT_BUFFER_INITIAL_SIZE);
    } else {
      ib->data = realloc(ib->data, size * sizeof(char));
    }
  }
  *(ib->data + ib->index++) = c;
}

void ibuffer_reset(struct ibuffer *ib) {
  ib->index = 0;
}

static int dict_scan_word(struct ibuffer* ib) {
  int cursor;
  ibuffer_reset(ib);
  while ((cursor = getc(ib->fd)) != EOF) {
    if (cursor == '\n') break;
    ibuffer_write(ib, cursor);
  }
  return ib->index;
}

int dict_load(struct dictionary* d, char* path) {
  // initialize struct
  d->count = 0;
  d->data = NULL;
  d->entries = NULL;

  // open dictionary file
  struct ibuffer ib;
  if (ibuffer_init(&ib, path) != 0) return errno;

  // scan words in dictionary files to find out the needed space in memory
  // and how many words there are.
  unsigned int size = 0;
  while (dict_scan_word(&ib) > 0) {
    size += ib.index + 1;
    d->count++;
  }
  d->data = malloc(size * sizeof(char));
  d->entries = malloc(d->count * sizeof(char*));
  *(d->entries) = d->data;

  // scan words again, this time we copy the words into memory in the
  // correct space assigned.
  rewind(ib.fd);
  unsigned int index = 0;
  while (dict_scan_word(&ib) > 0) {
    ibuffer_write(&ib, '\0');
    char* pos = d->entries[index++];
    memcpy(pos, ib.data, ib.index);
    d->entries[index] = pos + ib.index;
  }

  // close the dictionary file
  ibuffer_free(&ib);

  printf("<dictionary:count> %d\n", d->count);
  printf("<dictionary:size> %d\n", size);
  /*
  for (unsigned int i = 0; i < d->count; i++) {
    printf("[%p] (%d) %s\n", &(*d->entries[i]), i, d->entries[i]);
  }
  for (unsigned int i = 0; i < size; i++) {
    printf("[%p] (%d) %c\n", &d->data[i], i, d->data[i]);
  }
  */

  return EXIT_SUCCESS;
}

void dict_free(struct dictionary* d) {
  free(d->data);
  free(d->entries);
}