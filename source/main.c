#define _GNU_SOURCE 1
#include "crypto.h"
#include "dictionary.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <crypt.h>
#include <sys/random.h>
// #include <unistd.h>

void print_words(struct dictionary* d) {
	for (unsigned int i = 0; i < d->count; i++) {
		printf("<word:%d> %s\n", i, d->entries[i]);
	}
}

static char salt[13];
char saltchars[] = "abcdefghikjlmnopqrstuvWxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890+@£[]}";

struct queue {
	char** data;
	unsigned int offset;
	struct crypt_data crypt;
	char* hash;
};

void* dosleep(void* arg) {
	struct queue* q = (struct queue*) arg;
	// printf("START: %d %s\n", q->offset, *(q->data - q->offset));
	char* enc;
	for (int i = q->offset; i >= 0; i--) {
		enc = crypt_r(*(q->data - i), salt, &q->crypt);
		if (strncmp(enc, q->hash, 24) == 0) {
			printf("SUCCESS (%d) %s %s\n", i, *(q->data - i), enc);
		}
	}
	printf(" DONE: %d %s\n", q->offset, *(q->data - q->offset));
	pthread_exit(0);
}


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
	int num_threads = atoi(argv[3]);

	int error = EXIT_SUCCESS;
	struct dictionary d = { 0 };
	if ((error = dict_load(&d, dict_path)) != EXIT_SUCCESS) {
		dict_free(&d);
		return error;
	}

	/*
	pthread_t tid[4];
	pthread_attr_t attr[4];
	unsigned int time[4];
	for (int i = 0; i < 4; i++) {
	 	time[i] = (4 - i) * 3;
		pthread_attr_init(&attr[i]);
		pthread_create(&tid[i], &attr[i], dosleep, &time[i]);
	}
	for (int i = 0; i < 4; i++) {
		printf("Y %d\n", i);
		pthread_join(tid[i], NULL);
		printf("X %d\n", i);
	}
	printf("<done>\n");
	*/


	printf("<arg:dict> %s\n", dict_path);
	printf("<arg:pass> %s\n", pass);
	printf("<arg:nums> %d\n", num_threads);
	printf("<arg:hash> %s\n", hash);
	memcpy(&salt, hash, 12);
	salt[12] = '\0';
	printf("<salt> %s\n", salt);

	// init queue
	unsigned int queue_stride = 1000;
	unsigned int queue_size = 1 + ((d.count - 1) / queue_stride);
	struct queue* q = malloc(queue_size * sizeof(struct queue));
	for (unsigned int i = 0; i < queue_size; i++) {
		unsigned beg = i * queue_stride;
		unsigned end = beg + queue_stride - 1;
		if (end >= d.count) {
			end = d.count -1;
		}
		q[i].offset = end - beg;
		q[i].data = &d.entries[end];
		q[i].crypt.initialized = 0;
		q[i].hash = hash;
	}

	// print queue
	for (unsigned int i = 0; i < queue_size; i++) {
		// printf("%d %s\n", q[i].offset, *q[i].data);
		/*
		for (int j = q[i].offset; j >= 0; j--) {
			printf("(%d) %s\n", j, *(q[i].data - j));
		}
		*/
	}

	num_threads = queue_size;

	pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
	pthread_attr_t* attrs = malloc(num_threads * sizeof(pthread_attr_t));

	// attrs
	for (int i = 0; i < num_threads; i++) {
		pthread_attr_init(&attrs[i]);
	}

	// start
	for (int i = 0; i < num_threads; i++) {
		pthread_create(&threads[i], &attrs[i], dosleep, &q[i]);
	}

	// wait
	for (int i = 0; i < num_threads; i++) {
		pthread_join(threads[i], NULL);
	}
	printf("<done>\n");



	/*
	char *encrypted;
	int m;
	for (unsigned int i = 0, j = 0; i < d.count; i++, j++) {
		encrypted = crypt(d.entries[i], salt);
		m = strncmp(hash, encrypted, 34);
		if (m == 0) {
			printf("\n<SUCCESS>\n");
			printf("<encrypted> %s\n", encrypted);
			printf("<password> %s\n", d.entries[i]);
			break;
		}
		if (j > 10000) {
			printf(".");
			fflush(stdout);
			// printf("<%d / %d> %s %s %d %s\n", i, d.count, hash, encrypted, m, d.entries[i]);
			j = 0;
		}
	}
	*/


	/*
	printf("salt : %s\n", salt);
	char *encrypted;
	encrypted = crypt(d.entries[0], salt);
	printf("enc0 : %s\n", encrypted);
	printf("enc1 : %s\n", hash);
	printf("enc2 : %d\n", strncmp(encrypted, hash, 34));
	*/

	/*

	char test[35];
	memcpy(test, encrypted, 34);
	test[34] = '\0';

	encrypted = crypt("3636", salt);
	printf("enc1 : %s\n", encrypted);

	printf("? %d\n", strncmp(test, encrypted, 34));
	*/


	// printf("(%s == %s) = %d\n", hash, pass, strncmp(hash, pass, 8));
	// print_words(&d);

	/*
	char salt[13] = "$1$abcdefgh$";
	getrandom(salt + 3, 8, 0);
	for (unsigned int i = 3; i < 11; i++) {
		// salt[i] = saltchars[salt[i] & 0x3f];
	}
	char *encrypted = crypt(pass, salt);
	printf("> %s\n", encrypted);
	*/

	dict_free(&d);
	return error;
}