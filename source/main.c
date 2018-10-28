#include "tpool.h"
#include "wdictionary.h"
#include "solvers.h"
#include <stdlib.h>
#include <stdio.h>

static char* salt = "$1$7tBjugEa$";
static char* hash = "$1$7tBjugEa$h3cZLWYTXCwqikbFvQe7A/";

int main(int argc, char **argv) {
	// arguments and stuff
	if (argc != 4) return -1;
  char *dict_path = argv[2];
	char *password;

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
		printf("Found password: %s\n", password);
	}
  pthread_mutex_destroy(&dss.lock);

	// cleanup
	wdict_free(&wd);
	tpool_free(&tp);

	return EXIT_SUCCESS;
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