#define _GNU_SOURCE 1
#include "wsolver.h"
#include "tpool.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <crypt.h>
#include <pthread.h>

int wsolver_thread_worker(struct tpool* tp, struct tpool_message* msg, pthread_mutex_t* lock) {
  pthread_mutex_lock(lock);
  char* md5hash;
  struct crypt_data crypt;
	crypt.initialized = 0;
  do {
    struct wsolver_work* work = msg->arg;
    for (int i = 0; i < work->length; i += strlen(work->buffer + i) + 1) {
      md5hash = crypt_r(work->buffer + i, client_config.salt, &crypt);
      if (strncmp(md5hash, client_config.hash, 34) == 0) {
        work->pass = work->buffer + i;
        break;
      }
    }
    tpool_send(tp->queue_control, msg, TPOOL_WSOLVER_WORK, msg->arg, 2);
  } while (tpool_read(tp->queue_threads, msg) == TPOOL_WSOLVER_WORK);
  pthread_mutex_unlock(lock);
  return 0;
}