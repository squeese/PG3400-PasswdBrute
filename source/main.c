#include "tpool.h"
#include "wdictionary.h"
#include "wpermutation.h"
#include "solvers.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

// static char* salt = "$1$7tBjugEa$";
// static char* hash = "$1$7tBjugEa$h3cZLWYTXCwqikbFvQe7A/";
void test(const int n, const int k, const int offset) {
	// const int N = (int) pow((double) n, (double) k);
	/*
	if (offset == 0) {
		printf("0 [ -1 -1 -1 ]\n");
		return;
	}
	*/
	int* Y = malloc(k * sizeof(int));
	int r = offset;
	printf("%d [ ", offset);
	for (int x = k - 1; x >= 0; x--) {
		Y[x] = 0;
		int e = (int) pow((double) n, (double) x);
		int g = (r-0) / e;
		r -= g * e;
		// r -= v0;
		// r -= v0 * n;
		// printf(" (%d,%d,%d) ", x, e, g);
		printf(" %d ", g-1);
	}
	printf(" ]\n");
	// printf("f(n^k)\n");
	// printf("f(%d^%d) = %d\n", n, k, N);
	free(Y);
}

int main(int argc, char **argv) {
	// arguments and stuff
	if (argc != 4) return -1;

	struct wpermutation wp;
	const int LENGTH = 3;
	const int STRIDE = 1;
	wperm_init(&wp, LENGTH);
	// char words[] = "123456789";
	// char* words = "123456789";
	char* words = calloc((LENGTH+1)*STRIDE, sizeof(char));
	for (int i = 0; i < ((LENGTH+1)*STRIDE); i++) words[i] = '_';
	int n;
	int m = 0;
	int y = 0;
	while (1) {
		printf("%d [ %d %d %d ] ", y++, wp.y[0], wp.y[1], wp.y[2]);
		n = wperm_generate(&wp, words, STRIDE);
		if (n == 0) {
			printf("\n");
			break;
		}
		printf("%s (%d)\n", words, n);
		m += n;
	}
	wperm_free(&wp);
	free(words);

	for (int i = 0; i < 16; i++) test(3, LENGTH, i);

	struct wpermutation2 wp2;
	wperm_init2(&wp2, LENGTH);
	char* words2 = calloc((LENGTH + 1) * STRIDE, sizeof(char));
	for (int i = 0; i < 16; i++) {
		wperm_update(&wp2, i);
		wperm_generate2(&wp2, words2, STRIDE);
		printf("%d [", i);
		for (int j = 0; j < LENGTH; j++) {
			printf(" %d ", wp2.y[j]);
		}
		printf("] %s \n", words2);
	}



  // char *dict_path = argv[2];
	// char *password;

	// (how many letters) ^ (depth)

	// create thread pool
	// struct tpool tp;
	// tpool_init(&tp, 8);



	// load the password dictionary
	/*
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
	*/

	// cleanup
	// wdict_free(&wd);

	/*
	struct rand_solver_state = rss { 3, PTHREAD_MUTEX_INITIALIZER, salt, hash, NULL };
	if ((passord = tpool_run_solver(&tp, rand_solver_fn, &rss)) != NULL) {
		printf("Found password: %s\n", password);
	}
	rand_solver_free(&rss);
	*/

	// tpool_free(&tp);

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

/*
void gen(unsigned int depth) {
	unsigned int choices = sizeof(inputs) / sizeof(char);
	unsigned int combinations = pow((double) choices, (double) depth);
	// unsigned int index = 0;
	printf("%d %d = %d\n", depth, choices, combinations);

	char* buffer = calloc(depth + 1, sizeof(char));
	int* cols = malloc(depth * sizeof(int));
	for (unsigned int x = 0; x < depth; x++) {
		cols[x] = -1;
	}
	struct genstate gs = { depth, choices, inputs, buffer, cols, 0 };

	char* words = calloc(8 * (depth + 1), sizeof(char));
	walk(&gs, words);
	walk(&gs, words);
}
void wperm_generate(struct wpermutation* wp, char* words) {
	int* y = gs->cols;
	int x = gs->x;
	int Y = gs->n - 1;
	int X = gs->k;
	int size = sizeof(words) / sizeof(char);

	while (1) {
		if ((y[x] + 1) < Y) {
			y[x]++;
			gs->buffer[x] = gs->inputs[y[x]];
			if ((x + 1) < X) {
				x++;
			} else {
				printf("(%d, %d) ----------- %s\n", x, y[x], gs->buffer);
			}
		} else if((x - 1) >= 0) {
			y[x] = -1;
			x--;
		} else {
			printf("%d %d break\n", x, y[x]);
			break;
		}
	}
	gs->x = x;
}
*/