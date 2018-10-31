#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** args) {
  // if (argc == 1 && args[0] == "child_process")
  // close(1);
  // close(2);
  // fprintf(stdin, "client stdin\n");
  sleep(3);
  fprintf(stdout, "24500", argc);
  return 0;
}