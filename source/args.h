#ifndef BC_ARGS_H
#define BC_ARGS_H
// #include <sys/socket.h>
// #include <netinet/in.h>

const unsigned int ARGS_DEFAULT_SERVER_PORT;

struct args_server_config {
  char* host;
  unsigned int port;
  unsigned int threads;
  unsigned int stride;
};

int args_server_init(struct args_server_config*, int, char**);
void args_server_free(struct args_server_config*);

struct args_client_config {
  char* dictionary;
  int length;
  char salt[13];
  char hash[35];
  int num_servers;
  struct sockaddr** servers;
};

int args_client_init(struct args_client_config*, int, char**);
void args_client_push_server(struct args_client_config*, struct sockaddr*);
void args_client_free(struct args_client_config*);

#endif
