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

struct perm_solver_state {
	unsigned int word_size;
	unsigned int word_offset;
	pthread_mutex_t lock;
	char* salt;
	char* hash;
	char* pass;
};

void* dict_solver_fn(void*);
void* perm_solver_fn(void*);

#endif