#ifndef BC_WPERMUTATION_H
#define BC_WPERMUTATION_H

struct wpermutation {
	int x; // current x position (column selected)
	int X; // length of the word
	int* y; // current y values in each columns
	int Y; // length of the choices
	char* buffer; // the buffer to hold current constructed word
};

void wperm_init(struct wpermutation*, unsigned int);

int wperm_generate(struct wpermutation*, char*, unsigned int);

void wperm_free(struct wpermutation*);

struct wpermutation2 {
  int word_size;     // length of the word to generate
  int input_size;    // how many characters one can choose from when making a work
  int* y;
  int x;
  char* buffer;
};

void wperm_init2(struct wpermutation2*, int);
void wperm_update(struct wpermutation2*, unsigned int);
int wperm_generate2(struct wpermutation2*, char*, unsigned int);
void wperm_free2(struct wpermutation2*);

#endif
