#include "wpermutation.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

static const char inputs[] = "abc"; // de"; // fghik"; //jlmnopqrstuvwxyz"; //ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";//+@£[]}";

void wperm_init(struct wpermutation* wp, unsigned int word_length) {
	wp->x = 0;
	wp->X = word_length;
	wp->y = malloc(word_length * sizeof(int));
	wp->Y = sizeof(inputs) / sizeof(char) - 1;
	wp->buffer = calloc(word_length + 1, sizeof(char));
	for (unsigned int x = 0; x < word_length; x++)
		wp->y[x] = -1;
}

int wperm_generate(struct wpermutation* wp, char* words, unsigned int size) {
	unsigned int i = 0;
	while (i < size) {
		if ((wp->y[wp->x] + 1) < wp->Y) {
			wp->y[wp->x]++;
			wp->buffer[wp->x] = inputs[wp->y[wp->x]];
			if ((wp->x + 1) < wp->X) {
				wp->x++;
			} else {
				memcpy(words + i * (wp->X + 1), wp->buffer, wp->X);
 				i++;
			}
		} else if((wp->x - 1) >= 0) {
			wp->y[wp->x] = -1;
			wp->x--;
		} else break;
	}
	return i;
}

void wperm_free(struct wpermutation* wp) {
	free(wp->y);
	free(wp->buffer);
}

void wperm_init2(struct wpermutation2* wp, int word_size) {
  wp->word_size = word_size;
  wp->input_size = sizeof(inputs) / sizeof(char) - 1;
	wp->y = calloc(word_size, sizeof(int));
  wp->x = 0;
	wp->buffer = calloc(word_size + 1, sizeof(char));
}

void wperm_update(struct wpermutation2* wp, unsigned int offset) {
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

int wperm_generate2(struct wpermutation2* wp, char* words, unsigned int num) {
	unsigned int i = 0;
	while (i < num) {
		if ((wp->y[wp->x] + 1) < wp->input_size) {
			wp->y[wp->x]++;
			wp->buffer[wp->x] = inputs[wp->y[wp->x]];
			if ((wp->x + 1) < wp->word_size) {
				wp->x++;
			} else {
				memcpy(words + i * (wp->word_size + 1), wp->buffer, wp->word_size);
 				i++;
			}
		} else if((wp->x - 1) >= 0) {
			wp->y[wp->x] = -1;
			wp->x--;
		} else break;
	}
	return i;
}

void wperm_free2(struct wpermutation2* wp) {
  free(wp->y);
  free(wp->buffer);
}

/*
void test(const int n, const int k, const int offset) {
	const int N = (int) pow((double) n, (double) k);
	if (offset == 0) {
		printf("0 [ -1 -1 -1 ]\n");
		return;
	}
	int* Y = malloc(k * sizeof(int));
	int r = offset;
	printf("%d [ ", offset);
	for (int x = k - 1; x >= 0; x--) {
		Y[x] = 0;
		int e = (int) pow((double) n, (double) x);
		int g = (r-1) / e;
		r -= g * e;
		// r -= v0;
		// r -= v0 * n;
		// printf(" (%d,%d,%d) ", x, e, g);
		printf(" %d ", g);
	}
	printf(" ]\n");
	// printf("f(n^k)\n");
	// printf("f(%d^%d) = %d\n", n, k, N);
	free(Y);
}
*/
