#include "args.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/un.h>

static const unsigned int ARGS_DEFAULT_CLIENT_WORD_LENGTH_MIN = 1;
static const unsigned int ARGS_DEFAULT_CLIENT_WORD_LENGTH_MAX = 3;
static const unsigned int ARGS_DEFAULT_CLIENT_THREAD_BUFFER_SIZE = 512;
static const unsigned int ARGS_DEFAULT_CLIENT_THREAD_COUNT = 8;
static const char ARGS_DEFAULT_CLIENT_INPUT[] = "abcdefghijklmnopqrstvwxyzABCDEFGHIJKLMNOPQRSTVWXYZ0123456789";

static int args_client_info() {
  fprintf(stderr, "\n  OPTIONS:\n");
  fprintf(stderr, "    Predefined words in a dictionary file\n");
  fprintf(stderr, "    -d <path>                                ex: -d dictionary.txt\n");
  fprintf(stderr, "    Generated combinations of characters\n");
  fprintf(stderr, "    -l <n>     Length of words               ex: -l 3\n");
  fprintf(stderr, "    -L <n-m>   Length of words, in range     ex: -L 2-3\n");
  fprintf(stderr, "    -i <...c>  Input characters              ex: -i abcdefgh\n");
  fprintf(stderr, "    -I <n-m>   Input characters, in range    ex: -I A-Z -I a-z\n");
  fprintf(stderr, "    -C <n-m>   Same, but with ascii nrs      ex: -C 32-126\n");
  fprintf(stderr, "    -t <n>     Number of threads             ex: -t 8\n");
  fprintf(stderr, "    -b <n>     Char buffers size pr thread   ex: -b 512\n");
  fprintf(stderr, "  HASH:\n");
  fprintf(stderr, "    string of 34 characters\n");
  fprintf(stderr, "\n  Usage: client [...OPTIONS] HASH\n\n");
  return EXIT_FAILURE;
}

static void args_client_push_input(struct args_client_config* config, char c) {
  if (config->input_buffer == ARGS_DEFAULT_CLIENT_INPUT) {
    config->input_buffer = calloc(8, sizeof(char));
    config->input_length = 0;
  } else {
    // incrementally increase size by 8 as wee need it
    int cap = (1 + (config->input_length / 8)) * 8;
    if ((config->input_length / cap) != ((config->input_length + 1) / cap)) {
      config->input_buffer = realloc(config->input_buffer, cap + 8);
    }
  }
  *(config->input_buffer + (config->input_length++)) = c;
  *(config->input_buffer + config->input_length) = 0;
}

static void args_client_push_directory(struct args_client_config* config, char* path) {
  if (config->dictionary_paths == NULL) {
    config->dictionary_paths = malloc(sizeof(char*));
  } else {
    config->dictionary_paths = realloc(config->dictionary_paths, sizeof(char*) * (config->dictionary_count + 1));
  }
  *(config->dictionary_paths + (config->dictionary_count++)) = path;
}

int args_client_init(struct args_client_config* config, int argc, char** args) {
  config->dictionary_paths = NULL;
  config->dictionary_count = 0;
  config->input_buffer = (char*) ARGS_DEFAULT_CLIENT_INPUT;
  config->input_length = sizeof(ARGS_DEFAULT_CLIENT_INPUT) / sizeof(char) - 1;
  config->word_length_min = ARGS_DEFAULT_CLIENT_WORD_LENGTH_MIN;
  config->word_length_max = ARGS_DEFAULT_CLIENT_WORD_LENGTH_MAX;
  config->salt[12] = 0;
  config->hash[34] = 0;
  config->thread_buffer_size = ARGS_DEFAULT_CLIENT_THREAD_BUFFER_SIZE;
  config->thread_count = ARGS_DEFAULT_CLIENT_THREAD_COUNT;
  opterr = 0;
  int c;
  while ((c = getopt(argc, args, "d:l:L:i:I:t:b:C:")) != -1) {
    switch (c) {
      case 'd': {
        int len = strlen(optarg);
        char* path = malloc((len + 1) * sizeof(char));
        path[len] = 0;
        memcpy(path, optarg, len);
        args_client_push_directory(config, path);
        break;
      }
      case 'l': {
        unsigned int length = (unsigned int) atoi(optarg);
        if (length < 1) {
          args_client_info();
          fprintf(stderr, "  Invalid value for length (%d), cant be less than 1\n\n", length);
          return EXIT_FAILURE;
        }
        config->word_length_min = length;
        config->word_length_max = length;
        break;
      }
      case 'L': {
        int len = strlen(optarg);
        if (len < 3) {
          args_client_info();
          fprintf(stderr, "  Invalid value for -%c, format is N-M\n\n", optopt);
          return EXIT_FAILURE;
        }
        int dash_index = 1;
        for (; dash_index < (len - 1); dash_index++) {
          if (optarg[dash_index] == '-') break;
        }
        if (optarg[dash_index] != '-') {
          args_client_info();
          fprintf(stderr, "  Invalid value for -%c, format is N-M\n\n", optopt);
          return EXIT_FAILURE;
        }
        int min = atoi(optarg);
        int max = atoi(&optarg[dash_index + 1]);
        if (min <= 0 || max <= 0 || min > max) {
          args_client_info();
          fprintf(stderr, "  Invalid value for -%c, format is N-M, has to be positive values, and N must be less or equal to M\n\n", optopt);
          return EXIT_FAILURE;
        }
        config->word_length_min = min;
        config->word_length_max = max;
        break;
      }
      case 'i': {
        int len = strlen(optarg);
        for (int i = 0; i < len; i++) {
          args_client_push_input(config, optarg[i]);
        }
        break;
      }
      case 'I': {
        if (strlen(optarg) != 3 || optarg[1] != '-') {
          args_client_info();
          fprintf(stderr, "  Invalid value for -%c, format is A-Z\n\n", optopt);
          return EXIT_FAILURE;
        }
        int c_from = (int) optarg[0];
        int c_to = (int) optarg[2];
        if (c_from < 32 || c_from > 126 || c_to < 32 || c_to > 126 || c_to <= c_from) {
          args_client_info();
          fprintf(stderr, "  Invalid value for -%c, format is A-Z, A must be less than Z, A and Z must be valid ascii characters (32->126)\n\n", optopt);
          return EXIT_FAILURE;
        }
        for (; c_from <= c_to; c_from++) {
          args_client_push_input(config, (char) c_from);
        }
        break;
      }
      case 't': {
        int num = atoi(optarg);
        if (num < 1) {
          args_client_info();
          fprintf(stderr, "  Invalid value for thread-count (%d), cant be less than 1\n\n", num);
          return EXIT_FAILURE;
        }
        config->thread_count = num;
        break;
      }
      case 'b': {
        int num = atoi(optarg);
        if (num < 1) {
          args_client_info();
          fprintf(stderr, "  Invalid value for thread-buffer-size (%d), cant be less than 1\n\n", num);
          return EXIT_FAILURE;
        }
        config->thread_buffer_size = num;
        break;
      }
      case 'C': {
        int len = strlen(optarg);
        if (len < 3) {
          args_client_info();
          fprintf(stderr, "  Invalid value for -%c, format is N-M\n\n", optopt);
          return EXIT_FAILURE;
        }
        int dash_index = 1;
        for (; dash_index < (len - 1); dash_index++) {
          if (optarg[dash_index] == '-') break;
        }
        if (optarg[dash_index] != '-') {
          args_client_info();
          fprintf(stderr, "  Invalid value for -%c, format is N-M\n\n", optopt);
          return EXIT_FAILURE;
        }
        int min = atoi(optarg);
        int max = atoi(&optarg[dash_index + 1]);
        if (min <= 0 || max <= 0 || min > max) {
          args_client_info();
          fprintf(stderr, "  Invalid value for -%c, format is N-M, has to be positive values, and N must be less or equal to M\n\n", optopt);
          return EXIT_FAILURE;
        };
        for (; min <= max; min++) {
          args_client_push_input(config, (char) min);
        }
        break;
      }
      case '?':
        switch (optopt) {
          case 'd':
          case 'l':
          case 'L':
          case 'i':
          case 'I':
          case 't':
          case 'b':
          case 'C':
            args_client_info();
            fprintf(stderr, "  Option -%c requires an argument\n\n", optopt);
            return EXIT_FAILURE;
          default:
            args_client_info();
            fprintf(stderr, "  Option -%c is an invalid argument\n\n", optopt);
            return EXIT_FAILURE;
        }
        return args_client_info();
      default:
        return args_client_info();
    }
  }
  if ((argc - optind) != 1) {
    args_client_info();
    fprintf(stderr, "  Invalid number of arguments, missing hash value\n\n");
    return EXIT_FAILURE;
  }
  if (strlen(args[optind]) != 34) {
    args_client_info();
    fprintf(stderr, "  Invalid length of hash (%s), need to be 34 characters\n\n", args[optind]);
    return EXIT_FAILURE;
  }
  memcpy(config->salt, args[optind], 12);
  memcpy(config->hash, args[optind], 34);
  return EXIT_SUCCESS;
}

void args_client_free(struct args_client_config* config) {
  if (config->dictionary_paths != NULL) {
    for (int i = 0; i < config->dictionary_count; i++)
      free(*(config->dictionary_paths + i));
    free(config->dictionary_paths);
  }
  if (config->input_buffer != NULL && config->input_buffer != ARGS_DEFAULT_CLIENT_INPUT) {
    free(config->input_buffer);
    config->input_buffer = NULL;
  }
}