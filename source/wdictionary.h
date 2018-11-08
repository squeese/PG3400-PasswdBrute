#ifndef BC_WORD_DICTIONARY_H
#define BC_WORD_DICTIONARY_H
#include "args.h"
#include <stdio.h>

extern struct args_client_config client_config;

enum { WDICTIONARY_INITIAL_BUFFER_SIZE = 64 };
// The char* buffer points to the variable 'stack'.
// When its full, we alloc a new one on the heap,
// and point to that one instead. I bet 64 is plenty
// enough for most files (99% of cases).
struct wdictionary {
  FILE* fd;
  char stack[WDICTIONARY_INITIAL_BUFFER_SIZE];
  char* buffer;
  int index;    
  int size;
};

int wdictionary_init(struct wdictionary *wdict, char* path, long* size);
int wdictionary_fill(struct wdictionary* wdict, char* buffer, int cap);
void wdictionary_free(struct wdictionary* wdict);

#endif