#include "tpool.h"
#include <stdlib.h>

void tpool_init(struct tpool* tp, unsigned int count) {
  tp->count = count;
  tp->threads = malloc(count * sizeof(pthread_t));
}

char* tpool_run(struct tpool* tp, tpool_fn fn, void* state) {
  char* result = NULL;
  for (unsigned int i = 0; i < tp->count; i++) {
    pthread_create(&tp->threads[i], NULL, fn, state);
  }
  for (unsigned int i = 0; i < tp->count; i++) {
    void* tresult;
    pthread_join(tp->threads[i], &tresult);
    if (tresult != NULL) {
      result = (char*) tresult;
    }
  }
  return result;
}

void tpool_free(struct tpool* tp) {
  free(tp->threads);
}