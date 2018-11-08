#include "vmap.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <mqueue.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <math.h>
#include <stdio.h>

int vmap_load_file(struct vmap* vm, char* path) {
  int file = open(path, O_RDONLY, S_IRUSR | S_IWUSR);
  if (file == -1) {
    perror("open\n");
    return -1;
  }
  struct stat st;
  if (fstat(file, &st) == -1) {
    perror("fstat\n");
    return -1;
  }
  vm->address = mmap(NULL, st.st_size, PROT_WRITE, MAP_PRIVATE, file, 0);
  vm->size = st.st_size;
  // make absolutely sure the end of the pagefile is a \0 terminator, dont want to buffer overflow
  vm->address[st.st_size] = 0;
  return 0;
}