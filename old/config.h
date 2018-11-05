#ifndef BC_CONFIG_H
#define BC_CONFIG_H

struct cfg_client {
  char* host;
  int port;
  char* hash;
  char* dictionary;
  int threads;
  int stride;
  int depth;
};

typedef void*(*cfg_state_fn)(void *, char*);

void cfg_parse(void*, int, char**, cfg_state_fn);
int cfg_match(char*, unsigned int, ...);

void* cfg_client_root(void*, char*);
void* cfg_server_root(void*, char*);

#endif
