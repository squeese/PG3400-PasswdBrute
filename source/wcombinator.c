#include "wcombinator.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

/*
	Purpose of wcombinator is to generate all the possible words; given the
	length of the word, and the possible choices for characters (input).
	But the way its implemented here, is the ability to take a third parameter;
	an index as to which word.
	That means that its possible to get the N'th word in the set of combinations given
	the input; say, word length 3, characters 'abc', and it would always return the same.
	This was important in the initial implementation of this application, since each thread
	would generate and test words separately; becuase you could then only tell each thread to
	generate from N to M.
	Turns out it wasnt really necessary, since what actually takes time; is the hashing function. =)

	Anyway, the algo roughly is; wcomb_generate walks over 'the word' (char*) and increments
	the char's from left to right.

	And wcomb_update is able to calculate the state of that 'word' (char*) given the three inputs:
	word length, input length and an 'offset index'.
*/

long wcomb_init(struct wcombinator* wc, int word_size, char* inputs, int input_size) {
  wc->word_size = word_size;
	wc->inputs = inputs;
	wc->input_size = input_size;
	wc->buffer = calloc(word_size + 1, sizeof(char));
	wc->y = calloc(word_size, sizeof(int));
  wc->x = 0;
	wc->solutions = (long) pow((double) wc->input_size, (double) word_size);
 	return (long) wc->solutions * (word_size + 1);
}

void wcomb_update(struct wcombinator* wc, unsigned int offset) {
  double n = wc->input_size;
  int e, g, r = offset, k = wc->word_size - 1;
  for (int x = k; x >= 0; x--) {
    e = (int) pow(n, (double) x);
    g = r / e;
    r -= g * e;
    wc->y[k - x] = g - 1;
  }
  wc->x = 0;
}

int wcomb_generate(struct wcombinator* wc, char* words, unsigned int num) {
	unsigned int i = 0;
	while (i < num) {
		if ((wc->y[wc->x] + 1) < wc->input_size) {
			wc->y[wc->x]++;
			wc->buffer[wc->x] = wc->inputs[wc->y[wc->x]];
			if ((wc->x + 1) < wc->word_size) {
				wc->x++;
			} else {
				// copy the word over to the buffer
				memcpy(words + i * (wc->word_size + 1), wc->buffer, wc->word_size);
				// make sure it ends in a null terminator
				*(words + i * (wc->word_size + 1) + wc->word_size) = 0;
 				i++;
			}
		} else if((wc->x - 1) >= 0) {
			wc->y[wc->x] = -1;
			wc->x--;
		} else break;
	}
	return i * (wc->word_size + 1);
}

void wcomb_free(struct wcombinator* wc) {
	if (wc->buffer != NULL) {
  	free(wc->buffer);
		wc->buffer = NULL;
	}
	if (wc->y != NULL) {
		free(wc->y);
		wc->y = NULL;
	}
}