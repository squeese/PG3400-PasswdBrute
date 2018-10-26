#include "crypto.h"
#include "dictionary.h"
#include <stdlib.h>
#include <stdio.h>
// #include <stdlib.h>
// #include <crypt.h>
// #include <sys/random.h>

// char saltchars[] = "abcdefghikjlmnopqrstuvWxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890+@£[]}";
int main(int argc, char **argv) {
	if (argc != 3) return -1;

	char *passwd = argv[1];
	printf("<arg:pass> %s\n", passwd);

	char *dict_path = argv[2];
	printf("<arg:dict> %s\n", dict_path);

	// open file for reading
	// 

	int error = EXIT_SUCCESS;
	struct dictionary d = { 0 };
	if ((error = dict_load(&d, dict_path)) != EXIT_SUCCESS) {
		dict_free(&d);
		return error;
	}

	dict_free(&d);

	/*
	char salt[13] = "$1$abcdefgh$";
	getrandom(salt + 3, 8, 0);
	for (unsigned int i = 3; i < 11; i++) {
		salt[i] = saltchars[salt[i] & 0x3f];
	}

	char *encrypted = crypt(passwd, salt);
	printf("> %s\n", encrypted);
	*/
	return error;
}