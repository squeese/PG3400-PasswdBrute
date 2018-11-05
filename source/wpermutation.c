#include "wpermutation.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

static const char inputs[] = "abcfghikjlmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
// static const char inputs[]="_abcdefghikjlmnopqrstuvWxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!#$\%+-@£/|`'\"\\()[]{}";
// static const char inputs[]="aHi";
// static const char inputs[]="abcdefghikjlmnopqrstuvWxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890+@£[]}!#$\%^&*()";
// static const char inputs[] = "rstuwxyz1234567890"; // +@£[]}";
// static const char inputs[] = "0123456789";

void wperm_init(struct wpermutation* wp, int word_size) {
  wp->word_size = word_size;
  wp->input_size = sizeof(inputs) / sizeof(char) - 1;
	wp->buffer = calloc(word_size + 1, sizeof(char));
	wp->y = calloc(word_size, sizeof(int));
  wp->x = 0;
  int m = (int) pow((double) wp->input_size, (double) word_size);
  printf("num combinations: %d\n", m);
}

void wperm_update(struct wpermutation* wp, unsigned int offset) {
  double n = wp->input_size;
  int e, g, r = offset, k = wp->word_size - 1;
  for (int x = k; x >= 0; x--) {
    e = (int) pow(n, (double) x);
    g = r / e;
    r -= g * e;
    wp->y[k - x] = g - 1;
  }
  wp->x = 0;
}

int wperm_generate(struct wpermutation* wp, char* words, unsigned int num) {
	unsigned int i = 0;
	while (i < num) {
		if ((wp->y[wp->x] + 1) < wp->input_size) {
			wp->y[wp->x]++;
			wp->buffer[wp->x] = inputs[wp->y[wp->x]];
			if ((wp->x + 1) < wp->word_size) {
				wp->x++;
			} else {
				// copy the word over to the buffer
				memcpy(words + i * (wp->word_size + 1), wp->buffer, wp->word_size);
				// make sure it ends in a null terminator
				*(words + i * (wp->word_size + 1) + wp->word_size) = 0;
 				i++;
			}
		} else if((wp->x - 1) >= 0) {
			wp->y[wp->x] = -1;
			wp->x--;
		} else break;
	}
	return i * (wp->word_size + 1);
}

int wperm_buffer_size(struct wpermutation* wp, int num) {
	return sizeof(char) * (wp->word_size + 1) * num;
}

void wperm_free(struct wpermutation* wp) {
	if (wp->buffer != NULL) {
  	free(wp->buffer);
		wp->buffer = NULL;
	}
	if (wp->y != NULL) {
		free(wp->y);
		wp->y = NULL;
	}
}