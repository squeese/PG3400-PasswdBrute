#include "crypto.h"
#include "queue.h"
#include "dictionary.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

// char saltchars[] = "abcdefghikjlmnopqrstuvWxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890+@£[]}";

int main(int argc, char **argv) {
	if (argc != 4) return -1;
	// char *hash = "$1$9779ofJE$c.p.EwsI57yV2xjeorQbs1";
	char *hash = "$1$7tBjugEa$h3cZLWYTXCwqikbFvQe7A/"; // Great
	// char *hash = "$1$ivGZShhu$L/NLEkmbLWOSUTOm3cnmO/";
	// char *hash = "$1$K4BfHkEl$A1mF1S2ztQ7reX7GzWj7v0";
	// char *hash = "$1$RvQQ2SJN$Q80Nh4Ello9cx9Wllf5Nx/";
	// char *hash = "$1$CPqVGNrg$YH26ye4.Cft6c9AWf0zUn1";
	// char *hash = "$1$btfQSNEr$alX1tFUIDtW7bOfdDN2IK1";
	// char *hash = "$1$jgE1qlB8$l8.Oicr89Ib8DPhV/8nZp1";
	// char *hash = "$1$ckvWM6T@$H6H/R5d4a/QjpB02Ri/V01";
	// char *hash = "$1$9779ofJE$AGS41EkDh6j.usuCUld3a0"; // 3636
	char *pass = argv[1];
	char *dict_path = argv[2];
  printf("<arg:dict> %s\n", dict_path);
  printf("<arg:pass> %s\n", pass);
  printf("<arg:hash> %s\n", hash);

  printf("<dictionary:%s>\n", dict_path);
	int error = EXIT_SUCCESS;
	struct dictionary d;
	if ((error = dict_load(&d, dict_path)) != EXIT_SUCCESS) {
		dict_free(&d);
		return error;
	}

  struct queue q;
  if ((error = queue_init(&q, &d, hash)) != EXIT_SUCCESS) {
    queue_free(&q);
    return error;
  };
  printf("<queue:start>\n");
  queue_start(&q);
  printf("<queue:done>\n");

  // cleanup
  queue_free(&q);

	return error;
}
