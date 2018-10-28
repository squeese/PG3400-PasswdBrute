#ifndef BC_WDICTIONARY_H
#define BC_WDICTIONARY_H
#include <pthread.h>

struct wdictionary {
  unsigned int count;
  unsigned int index;
  unsigned int stride;
  char* words;
  char** word_indices;
};

struct wdictionary_slice {
  struct wdictionary* wd;
  unsigned int offset;
  char** word_indice;
};

int wdict_init(struct wdictionary*, unsigned int, char*);

int wdict_slice(struct wdictionary*, struct wdictionary_slice*);

void wdict_free(struct wdictionary*);

#endif
