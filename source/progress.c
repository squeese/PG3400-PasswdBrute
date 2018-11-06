#include "progress.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

void progress_init(struct progress* prog) {
  memset(prog, 0, sizeof(struct progress));
  prog->time_started = time(NULL);
  prog->val = 0;
  prog->max = 0;
  prog->title = prog->buffer + 32;
  for (int i = 0; i < 96; i++) *(prog->buffer + i) = ' ';
  printf("\n");
}

void progress_title(struct progress* prog, int length)  {    
  // clear the stuff after title
  if (length < 64) {
    memset(prog->title + length, 0, sizeof(char) * (64 - length));
  }
  *(prog->title + length) = '\r';
}

void progress_finish(struct progress* prog) {
  prog->val = prog->max * 0.97;
  progress_update(prog, 0);
  write(1, prog->buffer, 96);
}

void progress_update(struct progress* prog, long value) {
  double elapsed = time(NULL) - prog->time_started;
  prog->val += value;
  double spv = (double) prog->val / prog->max;
  time_t est = (elapsed / spv) - elapsed;
  strftime(prog->buffer, 12, " %2H:%2M:%2S ", localtime(&est));
  for (int i = 0; i < 20; i++) {
    *(prog->buffer + 11+i) = ((i/20.0) > spv) ? '-' : 'O';
  }
  write(1, prog->buffer, 96);
}
