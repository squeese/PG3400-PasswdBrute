#ifndef BC_VMAP_H
#define BC_VMAP_H
#include <sys/types.h>

struct vmap {
  char* address;
  size_t size;
};

int vmap_load_file(struct vmap* vm, char* path);

#endif