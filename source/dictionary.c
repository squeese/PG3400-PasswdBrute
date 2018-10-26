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
};

void ibuffer_init(struct ibuffer *ib) {
  ib->data = ib->stack;
  ib->index = 0;
  ib->size = INPUT_BUFFER_INITIAL_SIZE;
}

void ibuffer_free(struct ibuffer *ib) {
  if (ib->data == ib->stack) return;
  free(ib->data);
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

/*
typedef void (*scan_handler)(struct ibuffer*, void*, void*);

static void dict_scan(FILE* fd, struct ibuffer* ib, scan_handler handler, void* a, void* b) {
  int cursor;
  ibuffer_reset(ib);
  while ((cursor = getc(fd)) != EOF) {
    if (cursor == '\n') {
      handler(ib, a, b);
      ibuffer_reset(ib);
      continue;
    }
    ibuffer_write(ib, cursor);
  }
  if (ib->index > 0) handler(ib, a, b);
}

static void dict_scan_size(struct ibuffer* ib, void* size, void* count) {
  // d->size += ib->index + 1;
  // printf("dict_compute_size: (%d) (%d) %s\n", ib->index, d->size, ib->data);
  *((int*) size) += ib->index + 1;
  *((int*) count) += 1;
}

static void dict_scan_copy(struct ibuffer* ib, void* data, void* unused) {
  ibuffer_write(ib, '\0');
  char** entries = (char**) = data;
  // char* data = *((char**) entries) + (*(int*) index);
  // memcpy(data, ib->data, ib->index);
  // *((int*) index) += 1;
}
*/

static int dict_scan_word(FILE* fd, struct ibuffer* ib) {
  int cursor;
  ibuffer_reset(ib);
  while ((cursor = getc(fd)) != EOF) {
    if (cursor == '\n') break;
    ibuffer_write(ib, cursor);
  }
  return ib->index;
}

int dict_load(struct dictionary* d, char* path) {
  // struct dictionary* d = malloc(sizeof(struct dictionary));
  FILE* fd = fopen(path, "r");
  if (fd == NULL) {
    printf("Unable to open dictionary file at given path: %s\nError: (%d) %s\n", path, errno, strerror(errno));
    return errno;
  }

  // create file scan buffer

  d->count = 0;
  unsigned int size = 0;
  struct ibuffer ib = { 0 };
  ibuffer_init(&ib);
  while (dict_scan_word(fd, &ib) > 0) {
    size += ib.index + 1;
    d->count++;
  }
  d->data = malloc(size * sizeof(char));
  d->entries = malloc(d->count * sizeof(char*));
  *(d->entries) = d->data;
  rewind(fd);

  printf("<count> %d\n", d->count);
  
  unsigned int index = 0;
  while (dict_scan_word(fd, &ib) > 0) {
    ibuffer_write(&ib, '\0');
    char* pos = d->entries[index++];
    memcpy(pos, ib.data, ib.index);
    d->entries[index] = pos + ib.index;
  }
  fclose(fd);

  for (unsigned int i = 0; i < d->count; i++) {
    printf("[%p] (%d) %s\n", &(*d->entries[i]), i, d->entries[i]);
  }
  for (unsigned int i = 0; i < size; i++) {
    printf("[%p] (%d) %c\n", &d->data[i], i, d->data[i]);
  }



  // dict_scan(fd, &ib, &dict_store_entry)

  ibuffer_free(&ib);

  return EXIT_SUCCESS;
}

void dict_free(struct dictionary* d) {
  free(d->data);
  free(d->entries);
}