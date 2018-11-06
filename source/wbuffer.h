#ifndef BC_WBUFFER_H
#define BC_WBUFFER_H
#include <stdio.h>

// https://stackoverflow.com/questions/41946007/efficient-and-well-explained-implementation-of-a-quadtree-for-2d-collision-det

enum {
  WBUFFER_INITIAL_SIZE = 16
};

struct wbuffer {
  char stack[WBUFFER_INITIAL_SIZE];
  char* word;
  int index;
  int size;
  FILE* fd;
};

int wbuffer_init(struct wbuffer*, char*, long*);
void wbuffer_free(struct wbuffer*);
int wbuffer_fill(struct wbuffer*, char*, int);
int wbuffer_read(struct wbuffer*);
void wbuffer_write(struct wbuffer*, char);
void wbuffer_reset(struct wbuffer*);

#endif