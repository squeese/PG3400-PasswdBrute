#ifndef BC_PROGRESS_H
#define BC_PROGRESS_H
#include <time.h>

struct progress {
  time_t time_started;
  char buffer[96];
  char* title;
  long val;
  long max;
};

void progress_init(struct progress*);
void progress_title(struct progress*, int);
void progress_finish(struct progress*);
void progress_update(struct progress*, long);

#endif
