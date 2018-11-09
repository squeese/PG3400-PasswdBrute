#ifndef BC_ARGS_H
#define BC_ARGS_H

struct args_client_config {
  char salt[13];
  char hash[35];
  char** dictionary_paths;
  int dictionary_count;
  char* input_buffer;
  int input_length;
  int word_length_min;
  int word_length_max;
  int thread_buffer_size;
  int thread_count;
};

int args_client_init(struct args_client_config*, int, char**);
void args_client_free(struct args_client_config*);

#endif
