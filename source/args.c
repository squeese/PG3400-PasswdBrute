#include "args.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <arpa/inet.h>
// #include <sys/socket.h>
// #include <sys/types.h>
// #include <sys/stat.h>
#include <sys/un.h>
// #include <fcntl.h>

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

void args_client_push_server(struct args_client_config* config, struct sockaddr* server) {
  if (config->servers == NULL) {
    config->servers = malloc(sizeof(struct sockaddr*));
  } else {
    config->servers = realloc(config->servers, (config->num_servers + 1) * sizeof(struct sockaddr*));
  }
  config->servers[config->num_servers++] = server;
}

int args_client_init(struct args_client_config* config, int argc, char** args) {
  config->dictionary = (char*) ARGS_DEFAULT_CLIENT_DICTIONARY;
  config->length = ARGS_DEFAULT_CLIENT_LENGTH;
  config->salt[13] = 0;
  config->hash[34] = 0;
  config->num_servers = 0;
  config->servers = NULL;
  opterr = 0;
  int c;
  while ((c = getopt(argc, args, "d:l:s:")) != -1) {
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
      case 's': {
        int colon = -1;
        int len = strlen(optarg);
        for (int i = 0; i < len; i++) {
          if (optarg[i] == ':') {
            colon = i;
            break;
          }
        }
        printf("s: %s, colon: %d \n", optarg, colon);
        printf("v %d %ld\n", strncmp(optarg, "/tmp/", 5), strlen(optarg));
        if (colon != -1 && colon < (len - 1)) {
          // create 
          

        /*
        } else if (strncmp(optarg, "/tmp/", 5) == 0 && len > 6) {
          // Create a config to connect to a local server
          struct sockaddr_un* addr = malloc(sizeof(struct sockaddr_un));
          memset(addr, 0, sizeof(*addr));
          addr->sun_family = AF_UNIX;
          strncpy(addr->sun_path, optarg, sizeof(addr->sun_path) - 1);
          unlink(optarg);
          args_client_push(config, (struct sockaddr*) addr);
        */
        } else {
          // fprintf(stderr, "Option -%c requires a valid format. Either host:port or /sys/pathsocket\n", optopt);
          // return EXIT_FAILURE;
        }
        break;
      }
      case '?':
        switch (optopt) {
          case 'd':
          case 'l':
          case 's':
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
  memcpy(config->hash, args[optind], 34);
  return EXIT_SUCCESS;
}

void args_client_free(struct args_client_config* config) {
  if (config->dictionary != NULL && config->dictionary != ARGS_DEFAULT_CLIENT_DICTIONARY) {
    free(config->dictionary);
    config->dictionary = NULL;
  }
  if (config->servers != NULL) {
    for (int i = 0; i < config->num_servers; i++)
      free(*(config->servers + i));
    free(config->servers);
  }
}