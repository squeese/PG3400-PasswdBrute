#include "args.h"
#include "tqueue.h"
#include "tqueue_workers.h"
#include "wdictionary.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <mqueue.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <math.h>

          
struct args_client_config client_config;
// static char file[] = { 'h', 'e', 'l', '\r', '\n', 'l', 'o', '\r', '\n', 'l', 'o', 0 };
char* pagefile;

// static pthread_t a;
// static mqd_t queue;

/*
void* rootfn(void* arg) {
  return 0;
}
*/

void ddump(char** indices, int size) {
  int a = *(indices + size - 1) - *indices + strlen(*(indices + size - 1));
  int b = pagefile[]

  printf("dump %d %d\n", a, b);

  for (int i = 0; i < size && indices[i] != NULL; i++) {
    printf("%p %d `%s`\n", indices + i, (int)(indices + i), indices[i]);
  }
}


int main() {
  int fd = open("./misc/small.txt", O_RDONLY, S_IRUSR | S_IWUSR);
  struct stat sb;
  if (fstat(fd, &sb) == -1) perror("Filesize\n");
  printf("Filesize: %ld\n", sb.st_size);
  pagefile = mmap(NULL, sb.st_size, PROT_WRITE, MAP_PRIVATE, fd, 0);

  char** indices = malloc(32 * sizeof(char*));
  char* c = pagefile;


  do {
    int i = 0;
    for (; i < 32 && *c != 0; i++) {
      indices[i] = c;
      for (; *c != '\n' && *c != '\r'; c++)
        if (*c == 0) break;
      *c = 0;
      c++;
      for (; *c == '\n' || *c == '\r'; c++);
    }
    ddump(indices, i);
  } while(*c != 0);



  /*
    c = shared_memory + i;
    if (*c == '\n' || *c == '\r') {
      shared_memory[i] = 0;
      if (*(c + 1) == '\n') i++;

    }

      continue;
    }
    shared_memory[i] = 0;

  }
  char* buf = calloc(32, sizeof(char));
  memcpy(buf, file, 31);
  buf[31] = 0;
  printf("str: `%s`\n", buf);
  free(buf);
  */

  // pthread_create(&a, NULL, rootfn, NULL);
  // pthread_join(a, NULL);

/*
  mq_unlink("/testqueue");
  queue = mq_open("/testqueue", O_CREAT | O_RDWR, 0666, NULL);
  int x = 0;
  for (int i = 0; i < 10; i++) {
    mq_send(queue, (const char*) &x, sizeof(int), 0);
    printf("sent %d\n", i);
  }
  printf("setattr\n");
  struct mq_attr attr;
  attr.mq_flags = O_NONBLOCK;
  mq_setattr(queue, &attr, NULL);

  printf("take one value\n");
  mq_receive(queue, (char*) &x, sizeof(int), NULL);

*/

  printf("exit\n");
  return 0;
}