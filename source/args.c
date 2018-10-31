#include "args.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

const unsigned int ARGS_DEFAULT_SERVER_PORT = 56130;
static const unsigned int ARGS_DEFAULT_SERVER_THREADS = 8;
static const unsigned int ARGS_DEFAULT_SERVER_STRIDE = 64;
static const char ARGS_DEFAULT_CLIENT_DICTIONARY[] = "misc/small.txt";
static const unsigned int ARGS_DEFAULT_CLIENT_LENGTH = 5;

static int args_server_info() {
  fprintf(stderr, "Usage: server [...OPTIONS]\n");
  fprintf(stderr, " OPTIONS:      Description            Default\n");
  fprintf(stderr, "  -h hostname  server addres          0.0.0.0\n");
  fprintf(stderr, "  -p port      server port            %d\n", ARGS_DEFAULT_SERVER_PORT);
  fprintf(stderr, "  -t threads   number of threads      %d\n", ARGS_DEFAULT_SERVER_THREADS);
  fprintf(stderr, "  -s stride    data size per thread   %d\n", ARGS_DEFAULT_SERVER_STRIDE);
  return EXIT_FAILURE;
}

int args_server_init(struct args_server_config* config, int argc, char** args) {
  config->host = NULL;
  config->port = ARGS_DEFAULT_SERVER_PORT;
  config->threads = ARGS_DEFAULT_SERVER_THREADS;
  config->stride = ARGS_DEFAULT_SERVER_STRIDE;
  opterr = 0;
  int c;
  while ((c = getopt(argc, args, "h:p:t:s:")) != -1) {
    switch (c) {
      case 'h': {
        int len = strlen(optarg);
        config->host = malloc((len + 1) * sizeof(char));
        config->host[len] = 0;
        memcpy(config->host, optarg, len);
        break;
      }
      case 'p':
        config->port = (unsigned int) atoi(optarg);
        break;
      case 't': {
        unsigned int threads = (unsigned int) atoi(optarg);
        if (threads < 1) {
          fprintf(stderr, "Invalid value for port (%d), cant be less than 1\n", threads);
          return EXIT_FAILURE;
        }
        config->threads = threads;
        break;
      }
      case 's': {
        unsigned int stride = (unsigned int) atoi(optarg);
        if (stride < 1) {
          fprintf(stderr, "Invalid value for stride (%d), cabt be less than 1\n", stride);
          return EXIT_FAILURE;
        }
        config->stride = stride;
        break;
      }
      case '?':
        switch (optopt) {
          case 'h':
          case 'p':
          case 't':
          case 's':
            fprintf(stderr, "Option -%c requires an argument\n", optopt);
            break;
          default:
            fprintf(stderr, "Option -%c is an invalid argument\n", optopt);
        }
        return args_server_info();
      default:
        return args_server_info();
    }
  }
  return EXIT_SUCCESS;
}

void args_server_free(struct args_server_config* config) {
  if (config->host != NULL) {
    free(config->host);
    config->host = NULL;
  }
}

static int args_client_info() {
  fprintf(stderr, "Usage: client [...OPTIONS] HASH\n");
  fprintf(stderr, " OPTIONS:    Description                     Default\n");
  fprintf(stderr, "  -d         path to dictionary file         %s\n", ARGS_DEFAULT_CLIENT_DICTIONARY);
  fprintf(stderr, "  -l         max word size when guessing     %d\n", ARGS_DEFAULT_CLIENT_LENGTH);
  fprintf(stderr, " HASH:\n");
  fprintf(stderr, "  * required\n");
  fprintf(stderr, "  * must be a string of 34 characters\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Examples:\n");
  fprintf(stderr, " solve hash on local machine\n");
  fprintf(stderr, "  client $1$ckvWM6T@$H6H/R5d4a/QjpB02Ri/V01\n");
  fprintf(stderr, " solve hash on a remote machine\n");
  fprintf(stderr, "  client -h 192.168.1.2 -p 5000 $1$ckvWM6T@$H6H/R5d4a/QjpB02Ri/V01\n");
  return EXIT_FAILURE;
}

int args_client_init(struct args_client_config* config, int argc, char** args) {
  config->dictionary = (char*) ARGS_DEFAULT_CLIENT_DICTIONARY;
  config->length = ARGS_DEFAULT_CLIENT_LENGTH;
  config->salt[12] = 0;
  config->hash[22] = 0;
  config->num_servers = 0;
  config->servers = NULL;
  opterr = 0;
  int c;
  while ((c = getopt(argc, args, "d:l:")) != -1) {
    switch (c) {
      case 'd': {
        int len = strlen(optarg);
        config->dictionary = malloc((len + 1) * sizeof(char));
        config->dictionary[len] = 0;
        memcpy(config->dictionary, optarg, len);
        break;
      }
      case 'l': {
        unsigned int length = (unsigned int) atoi(optarg);
        if (length < 1) {
          fprintf(stderr, "Invalid value for length (%d), cant be less than 1\n", length);
          return EXIT_FAILURE;
        }
        config->length = length;
        break;
      }
      case '?':
        switch (optopt) {
          case 'd':
          case 'l':
            fprintf(stderr, "Option -%c requires an argument\n", optopt);
            break;
          default:
            fprintf(stderr, "Option -%c is an invalid argument\n", optopt);
        }
        return args_client_info();
      default:
        return args_client_info();
    }
  }
  if ((argc - optind) != 1) {
    fprintf(stderr, "Invalid number of arguments\n");
    return args_client_info();
  }
  if (strlen(args[optind]) != 34) {
    fprintf(stderr, "Invalid length of hash (%s), need to be 34 characters\n", args[optind]);
    return args_client_info();
  }
  memcpy(config->salt, args[optind], 12);
  memcpy(config->hash, args[optind] + 12, 22);
  return EXIT_SUCCESS;
}

void args_client_free(struct args_client_config* config) {
  if (config->dictionary != NULL && config->dictionary != ARGS_DEFAULT_CLIENT_DICTIONARY) {
    free(config->dictionary);
    config->dictionary = NULL;
  }
}