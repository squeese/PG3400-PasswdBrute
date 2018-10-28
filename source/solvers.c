#define _GNU_SOURCE 1
#include "solvers.h"
#include "wdictionary.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <crypt.h>

void* dict_solver_fn(void* arg) {
	static unsigned int ID = 0;
	unsigned int id = ID++;
	printf("<thread:%d>\n", id);
	struct dict_solver_state* state = arg;
	struct wdictionary_slice wds;
  char* encoded;
  struct crypt_data crypt;
	crypt.initialized = 0;
	while (state->pass == NULL) {
  	pthread_mutex_lock(&state->lock);
		if (wdict_slice(state->wd, &wds) != EXIT_SUCCESS) {
  		pthread_mutex_unlock(&state->lock);
			break;
		}
  	pthread_mutex_unlock(&state->lock);
		for (int i = wds.offset; i >= 0; i--) {
			encoded = crypt_r(*(wds.word_indice - i), state->salt, &crypt);
			if (strncmp(encoded, state->hash, 34) == 0) {
				printf("<thread:%d> SUCCESS (%d) %s %s\n", id, i, *(wds.word_indice - i), encoded);
				// pthread_exit(*(wds.word_indice - i));
  			pthread_mutex_lock(&state->lock);
				state->pass = *(wds.word_indice - i);
  			pthread_mutex_unlock(&state->lock);
  			pthread_exit(state->pass);
			}
		}
	}
	printf("</thread:%d>\n", id);
  pthread_exit(0);
}
