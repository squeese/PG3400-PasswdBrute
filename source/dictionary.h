#ifndef BC_DICTIONARY_H
#define BC_DICTIONARY_H

struct dictionary {
  unsigned int count;
  char** entries;
  char* data;
};

int dict_load(struct dictionary*, char*);

void dict_free(struct dictionary*);

#endif