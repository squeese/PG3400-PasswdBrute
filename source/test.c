#include "wpermutation.h"
#include "args.h"
#include "tpool.h"
#include <stdio.h>
#include <stdlib.h>

struct tpool_queue queue;
struct args_client_config client_config;

int main() {
  struct wpermutation wp;
  wperm_init(&wp, 3);
  wperm_update(&wp, 0);

  char* buffer = malloc(wperm_buffer_size(&wp, 128));
  int l = wperm_generate(&wp, buffer, 128);
  for (int i = 0; i < l; i++) {
    if (*(buffer + i) == 0) {
      *(buffer + i) = '_';
    }
  }
  printf("%d `%s`\n", l, buffer);
  printf("%d\n", wperm_buffer_size(&wp, 128));

  return 0;
}