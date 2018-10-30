#ifndef BC_ARGS_H
#define BC_ARGS_H

struct args_server_config {
  char* host;
  unsigned int port;
  unsigned int threads;
  // char salt[13];
  // char hash[23];
};

int args_server_init(struct args_server_config*, int, char**);
void args_server_free(struct args_server_config*);

#endif
