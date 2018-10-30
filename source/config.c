#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

void* cfg_client_dictionary(void* cfg, char* arg) {
  ((struct cfg_client*) cfg)->dictionary = arg;
  return &cfg_client_root;
}

void* cfg_client_threads(void* cfg, char* arg) {
  ((struct cfg_client*) cfg)->threads = atoi(arg);
  return &cfg_client_root;
}

void* cfg_client_stride(void* cfg, char* arg) {
  ((struct cfg_client*) cfg)->stride = atoi(arg);
  return &cfg_client_root;
}

void* cfg_client_depth(void* cfg, char* arg) {
  ((struct cfg_client*) cfg)->depth = atoi(arg);
  return &cfg_client_root;
}

void* cfg_client_host(void* cfg, char* arg) {
  ((struct cfg_client*) cfg)->host = arg;
  return &cfg_client_root;
}

void* cfg_client_port(void* cfg, char* arg) {
  ((struct cfg_client*) cfg)->port = atoi(arg);
  return &cfg_client_root;
}

void* cfg_client_root(void* cfg, char* arg) {
  if (cfg_match(arg, 2, "-d", "--dictionary")) return &cfg_client_dictionary;
  if (cfg_match(arg, 2, "-t", "--threads")) return &cfg_client_threads;
  if (cfg_match(arg, 2, "-s", "--stride")) return &cfg_client_stride;
  if (cfg_match(arg, 2, "-d", "--depth")) return &cfg_client_depth;
  if (cfg_match(arg, 2, "-h", "--host")) return &cfg_client_host;
  if (cfg_match(arg, 2, "-p", "--port")) return &cfg_client_port;
  if (strlen(arg) == 34) ((struct cfg_client*) cfg)->hash = arg;
  else printf("Unknown config parameter: %s\n", arg);
  return NULL;
}

void cfg_parse(void* config, int argc, char** args, cfg_state_fn fn) {
  for (int i = 1; i < argc && fn != NULL; i++) {
    fn = (cfg_state_fn)(*fn)(config, args[i]);
  }
}

int cfg_match(char* arg, unsigned int num, ...) {
  va_list args;
  va_start(args, num);
  for (unsigned int i = 0; i < num; i++) {
    char* val = va_arg(args, char*);
    if (strncmp(arg, val, strlen(val)) == 0) return 1;
  }
  va_end(args);
  return 0;
}
