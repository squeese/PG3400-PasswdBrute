#ifndef BC_SOLVERS_H
#define BC_SOLVERS_H
#include <pthread.h>

struct dict_solver_state {
	struct wdictionary* wd;
  pthread_mutex_t lock;
	char* salt;
	char* hash;
	char* pass;
};

/*
struct rand_solver_state {
	unsigned int size;
	pthread_mutex_t lock;
}
*/

void* dict_solver_fn(void*);

/*
void* rand_solver_fn(void*);
void rand_solver_free(struct rand_solver_state* rss);
*/

#endif