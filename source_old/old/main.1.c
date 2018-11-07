#include "tpool.h"
#include "wdictionary.h"
#include "wpermutation.h"
#include "solvers.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

static char salt[13];
// static char* hash = "$1$7tBjugEa$h3cZLWYTXCwqikbFvQe7A/"; // Great
static char* hash = "$1$9779ofJE$AGS41EkDh6j.usuCUld3a0"; // 3636

int main(int argc, char **argv) {
	// arguments and stuff
	if (argc != 4) return -1;

  char *dict_path = argv[2];
	char *password;
	memcpy(salt, hash, 12);
	salt[12] = 0;

	// create thread pool
	struct tpool tp;
	tpool_init(&tp, 8);

	// load the password dictionary
	int error;
	struct wdictionary wd;
	if ((error = wdict_init(&wd, 32, dict_path)) != EXIT_SUCCESS) {
		wdict_free(&wd);
		tpool_free(&tp);
		return error;
	}

	// try to solve the hash with a dictionary
	struct dict_solver_state dss = { &wd, PTHREAD_MUTEX_INITIALIZER, salt, hash, NULL };
	if ((password = tpool_run_solver(&tp, dict_solver_fn, &dss)) != NULL) {
		printf("Dictionary -> Found password: %s\n", password);
	} 
  pthread_mutex_destroy(&dss.lock);

	// try to solve the hash with a string permutation
	const int LENGTH = 4;
	const int STRIDE = 128;
	struct perm_solver_state pss = { LENGTH, 0, PTHREAD_MUTEX_INITIALIZER, salt, hash, NULL };
	if ((password = tpool_run_solver(&tp, perm_solver_fn, &pss)) != NULL) {
		printf("Permutation -> Found password: %s\n", password);
	} 
  pthread_mutex_destroy(&pss.lock);

	// cleanup
	wdict_free(&wd);

	return EXIT_SUCCESS;
}