#include "args.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

static int args_server_info() {
  fprintf(stderr, "Usage: [...options] hash\n");
  fprintf(stderr, "Options:      default    description\n");
  fprintf(stderr, " -h hostname  localhost  server addres\n");
  fprintf(stderr, " -p port      65999      server port\n");
  fprintf(stderr, " -t threads   8          number of threads\n");
  return EXIT_FAILURE;
}

int args_server_init(struct args_server_config* config, int argc, char** args) {
  config.host = NULL;
  config.port = 65999;
  config.threads = 8;
  opterr = 0;
  int c;
  while ((c = getopt(argc, args, "x:y:z:h")) != -1) {
    switch (c) {
      case 'h':
        int len = strlen(optvar);
        config.host = malloc((len + 1) * sizeof(char));
        config.host[len] = 0;
        memcpy(config.host, optvar, len);
        break;
      case 'p':
        config.port = (unsigned int) atoi(optvar);
        break;
      case 't':
        unsigned int threads = (unsigned int) atoi(optvar);
        if (threads < 1) {
          fprintf(stderr, "Invalid value for port (%d), cant be less than 1\n", threads);
          return EXIT_FAILURE;
        }
        config.threads = t;
        break;
      case '?':
        switch (optopt) {
          case 'h':
          case 'p':
          case 't':
            fprintf(stderr, "Option -%c requires an argument\n", optopt);
            break;
          default:
            fprintf(stderr, "Option -%c is an invalid argument\n", optopt);
        }
        break;
      default:
        return args_server_info();
    }
  }
  if ((argc - optind) != 1) {
    return args_server_info();
  }
  if (strlen(args[optind]) != 34) {
    fprintf(stderr, "Invalid length of hash (%s), need to be 34 characters\n", args[optind]);
    return args_server_info();
  }
  memcpy(config.salt, args[optind], 12);
  memcpy(config.hash, args[optind], 34);

  printf("Server config:\n");
  printf(" host    : %s\n", config.host == NULL ? "localhost" : config.host);
  printf(" port    : %d\n", config.port);
  printf(" threads : %d\n", config.threads);
  printf(" salt    : %s\n", config.salt);
  printf(" hash    : %s\n", config.hash);

  return EXIT_SUCCESS;
}

void args_server_free(struct args_server_config* config) {
  if (config.host != NULL) {
    free(config.host);
  }
}