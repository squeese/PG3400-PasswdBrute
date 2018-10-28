#define _GNU_SOURCE 1
#include "wdictionary.h"
#include "tpool.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <crypt.h>
#include <unistd.h>

static char* salt = "$1$7tBjugEa$";
static char* hash = "$1$7tBjugEa$h3cZLWYTXCwqikbFvQe7A/";

struct dict_solver_state {
	struct wdictionary* wd;
  pthread_mutex_t lock;
	char* salt;
	char* hash;
};

void* dict_solver_fn(void* arg) {
	static unsigned int ID = 0;
	unsigned int id = ID++;
	printf("<thread:%d>\n", id);

	struct dict_solver_state* state = arg;
	struct wdictionary_slice wds;
  char* encoded;
  struct crypt_data crypt;
	crypt.initialized = 0;
	while (wdict_slice(state->wd, &wds) == EXIT_SUCCESS) {
		for (int i = wds.offset; i >= 0; i--) {
			encoded = crypt_r(*(wds.word_indice - i), state->salt, &crypt);
			if (strncmp(encoded, state->hash, 34) == 0) {
				printf("<thread:%d> SUCCESS (%d) %s %s\n", id, i, *(wds.word_indice - i), encoded);
				pthread_exit(*(wds.word_indice - i));
			}
		}
	}

	printf("</thread:%d>\n", id);
  pthread_exit(0);
}

void* crack(void* arg, unsigned int* alive) {
	static unsigned int idc = 0;
	unsigned int id = idc++;
	printf("<thread:%d>\n", id);
	sleep(1);
	struct wdictionary* wd = (struct wdictionary*) arg;
	struct wdictionary_slice wds;
  char* encoded;
  struct crypt_data crypt;
	crypt.initialized = 0;
	while (*alive && wdict_slice(wd, &wds) == EXIT_SUCCESS) {
		// printf("<thread:%d> slice (%d)\n", id, wds.offset);
		for (int i = wds.offset; i >= 0; i--) {
			// printf("<thread:%d> process (%d) %s\n", id, i, *(wds.word_indice - i));
			encoded = crypt_r(*(wds.word_indice - i), salt, &crypt);
			if (strncmp(encoded, hash, 34) == 0) {
				printf("<thread:%d> SUCCESS (%d) %s %s\n", id, i, *(wds.word_indice - i), encoded);
				*alive = 0;
				pthread_exit(*(wds.word_indice - i));
			}
		}
	}
	printf("</thread:%d>\n", id);
  pthread_exit(0);
}

int main(int argc, char **argv) {
	if (argc != 4) return -1;
  char *dict_path = argv[2];

	int error = EXIT_SUCCESS;
	struct wdictionary wd;
	if ((error = wdict_init(&wd, 32, dict_path)) != EXIT_SUCCESS) {
		wdict_free(&wd);
		return error;
	}

	// create thread pool
	struct tpool tp;
	tpool_init(&tp, 8);

	struct dict_solver_state dss = { &wd, PTHREAD_MUTEX_INITIALIZER, salt, hash };
	tpool_run_solver(&tp, dict_solver_fn, &dss);
	wdict_free(&wd);
	tpool_free(&tp);

	return error;
}

// char *hash = "$1$9779ofJE$c.p.EwsI57yV2xjeorQbs1";
// char *hash = "$1$ivGZShhu$L/NLEkmbLWOSUTOm3cnmO/";
// char *hash = "$1$K4BfHkEl$A1mF1S2ztQ7reX7GzWj7v0";
// char *hash = "$1$RvQQ2SJN$Q80Nh4Ello9cx9Wllf5Nx/";
// char *hash = "$1$CPqVGNrg$YH26ye4.Cft6c9AWf0zUn1";
// char *hash = "$1$btfQSNEr$alX1tFUIDtW7bOfdDN2IK1";
// char *hash = "$1$jgE1qlB8$l8.Oicr89Ib8DPhV/8nZp1";
// char *hash = "$1$ckvWM6T@$H6H/R5d4a/QjpB02Ri/V01";
// char *hash = "$1$9779ofJE$AGS41EkDh6j.usuCUld3a0"; // 3636
// printf("<arg:dict> %s\n", dict_path);
// printf("<arg:pass> %s\n", pass);
// printf("<arg:hash> %s\n", hash);
// char *pass = argv[1];