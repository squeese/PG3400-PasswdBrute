#ifndef BC_WPERMUTATION_H
#define BC_WPERMUTATION_H

struct wpermutation {
  char* inputs;
  int word_size;     // length of the word to generate
  int input_size;    // how many characters one can choose from when making a work
  char* buffer;      // the current word being assembled
  int* y;            // ?
  int x;             // yes
  long solutions;
};

void wperm_init(struct wpermutation*, int, char*, int, long*);
void wperm_update(struct wpermutation*, unsigned int);
int wperm_generate(struct wpermutation*, char*, unsigned int);
void wperm_free(struct wpermutation*);

#endif
