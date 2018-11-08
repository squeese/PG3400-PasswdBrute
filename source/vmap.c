#include "pagefile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int pagefile_load(struct pagefile* pf, char* path) {
  int file_index = open(path, O_RDONLY, S_IRUSR | S_IWUSR)
  if (file_index == -1) {
    perror("Open: %s\n", path);
    return -1;
  }
  struct stat file_stat;
  if (fstat(file_index, &file_stat) == -1) {
    perror("Filesize: %s\n", path);
    return -1;
  }
  pf->address = pagefile = mmap(NULL, file_stat.st_size, PROT_WRITE, MAP_PRIVATE, file_index, 0);
  pf->size = file_state.st_size;
  pf->processed = 0;
  pf->pending = 0;
  // make absolutely sure the end of the pagefile is a \0 terminator, dont want to buffer overflow
  pf->address[sb.st_size] = 0;
  return 0;
}