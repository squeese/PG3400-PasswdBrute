#include "wpermutation.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

void wperm_init(struct wpermutation* wp, int word_size, char* inputs, int input_size, long* total) {
  wp->word_size = word_size;
	wp->inputs = inputs;
	wp->input_size = input_size;
	wp->buffer = calloc(word_size + 1, sizeof(char));
	wp->y = calloc(word_size, sizeof(int));
  wp->x = 0;
	wp->solutions = (long) pow((double) wp->input_size, (double) word_size);
	if (total != NULL) {
  	*total = wp->solutions * (word_size + 1);
	}
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
			wp->buffer[wp->x] = wp->inputs[wp->y[wp->x]];
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