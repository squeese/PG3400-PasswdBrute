#ifndef BC_VMAP_H
#define BC_VMAP_H

struct task {
  char* vmap_address;
  
};

struct vmap {
  char* address = NULL;
  size_t size;
};

int vmap_load(struct vmap* vm, char* path);

#endif
