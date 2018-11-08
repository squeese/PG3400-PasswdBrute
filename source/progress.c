#include "progress.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

void progress_init(struct progress* prog, long max) {
  memset(prog, 0, sizeof(struct progress));
  prog->time_started = time(NULL);
  prog->val = 0;
  prog->max = max;
  prog->title = prog->buffer + 32;
  for (int i = 0; i < 96; i++) *(prog->buffer + i) = ' ';
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
  // -Wunused-result ?
  // Uhm, I guess we make sure it writes the amount of characters
  // its told?
  int len = write(1, prog->buffer, 96);
  // *note: is it actually possible to not be able to write specified
  // amount to stdout?
  if (len != 96) fprintf(stderr, "write wrote %d, instead of 96\n", len);
}

void progress_update(struct progress* prog, long value) {
  double elapsed = time(NULL) - prog->time_started;
  prog->val += value;
  double spv = (double) prog->val / prog->max;
  time_t est = (elapsed / spv) - elapsed;
  strftime(prog->buffer, 14, "> %2H:%2M:%2S :", localtime(&est));
  for (int i = 0; i < 17; i++) {
    *(prog->buffer + 14 + i) = ((i / 20.0) > spv) ? '-' : 'O';
  }
  int len = write(1, prog->buffer, 96);
  if (len != 96) fprintf(stderr, "write wrote %d, instead of 96\n", len);
}
