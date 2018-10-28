#include "wdictionary.h"
#include "wbuffer.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>


int wdict_init(struct wdictionary* wd, unsigned int stride, char* path) {
  // initialize struct
  wd->count = 0;
  wd->index = 0;
  wd->stride = stride;
  wd->words = NULL;
  wd->word_indices = NULL;

  // initialize mutex lock
  if (pthread_mutex_init(&wd->lock, NULL) != 0) {
    printf("Unable to initate mutex lock.\nError: (%d) %s\n", errno, strerror(errno));
    return errno;
  }

  // open dictionary file
  struct wbuffer wb;
  if (wbuffer_init(&wb, path) != 0) return errno;

  // scan words in dictionary files to find out the needed space in memory
  // and how many words there are.
  unsigned int size = 0;
  while (wbuffer_read(&wb) > 0) {
    size += wb.index + 1;
    wd->count++;
  }
  wd->words = malloc(size * sizeof(char));
  wd->word_indices = malloc(wd->count * sizeof(char*));
  *(wd->word_indices) = wd->words;

  // scan words again, this time we copy the words into memory in the
  // correct space assigned.
  rewind(wb.fd);
  unsigned int index = 0;
  while (wbuffer_read(&wb) > 0) {
    wbuffer_write(&wb, '\0');
    char* pos = wd->word_indices[index++];
    memcpy(pos, wb.word, wb.index);
    wd->word_indices[index] = pos + wb.index;
  }

  // close the dictionary file
  wbuffer_free(&wb);

  return EXIT_SUCCESS;
}

int wdict_slice(struct wdictionary* wd, struct wdictionary_slice* wds) {
  pthread_mutex_lock(&wd->lock);
  if (wd->index >= wd->count) {
    pthread_mutex_unlock(&wd->lock);
    return EXIT_FAILURE;
  }
  unsigned int word_start = wd->index;
  unsigned int word_end = word_start + wd->stride;
  if (word_end >= wd->count) {
    word_end = wd->count - 1;
    wd->index++;
  }
  wds->offset = word_end - word_start;
  wd->index += wds->offset;
  pthread_mutex_unlock(&wd->lock);
  wds->word_indice = &wd->word_indices[word_end];
  return EXIT_SUCCESS;
}

void wdict_free(struct wdictionary* wd) {
  pthread_mutex_destroy(&wd->lock);
  free(wd->words);
  free(wd->word_indices);
}