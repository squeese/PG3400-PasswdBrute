#ifndef BC_WORD_COMBINATOR_H
#define BC_WORD_COMBINATOR_H

struct wcombinator {
  char* inputs;
  int word_size;     // length of the word to generate
  int input_size;    // how many characters one can choose from when making a work
  char* buffer;      // the current word being assembled
  int* y;            // ?
  int x;             // yes
  long solutions;
};

void wcomb_init(struct wcombinator*, int, char*, int, long*);
void wcomb_update(struct wcombinator*, unsigned int);
int wcomb_generate(struct wcombinator*, char*, unsigned int);
void wcomb_free(struct wcombinator*);

#endif
