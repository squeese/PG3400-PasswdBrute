#define _GNU_SOURCE 1
#include "tqueue_workers.h"
#include "wdictionary.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <crypt.h>
#include <pthread.h>
#include <time.h>
#include <mqueue.h>

/*
  The main 'pthread routine', its job is to read from the threads channel and
  delegate the messages appropriately.
*/
void tqueue_worker_root(mqd_t threads, mqd_t control, struct tqueue_message* msg) {
  // pthread_detach(pthread_self());
  // Send message to 'control' that were running, and register a cleanup
  // function to send a message that were done.
  tqueue_send(control, msg, TQMESSAGE_PUSH, NULL, 0, 3);
  pthread_cleanup_push(&tqueue_worker_root_cleanup, &control);

  // Read from the theads channel and process
  while (tqueue_read(threads, msg)) {

    if (TQMESSAGE_TEST_WORDS == msg->flag) {
      // the message has a buffer if N size (defined by user terminal input)
      // that contains words, separated by \0 terminator.
      // we 'defer control' of the thread to this worker, it will read messages
      // until it finds one it wont handle
      tqueue_worker_word_tester(threads, control, msg);
      // we dont issue continue on the while loop here and read a new message,
      // we just let it fall through; since word_tester fn exits when it reads
      // a non TQMESSAGE_TEST_WORDS message, so someone else needs to handle it
    }

    if (TQMESSAGE_RETURN & msg->flag) {
      // return flag indicates that control thread (main) wants a flag
      // sent to itself, its a work around for deadlock.
      // So we mask away the return flag from the original
      tqueue_send(control, msg, msg->flag & ~TQMESSAGE_RETURN, msg->arg, msg->argn, 1);
      continue;
    }

    if (TQMESSAGE_WDICTIONARY == msg->flag) {
      // a worker who reads words from a file, and queues a message to try
      // and match any of the words against the md5hash we got, to try find
      // password
      tqueue_worker_wdictionary(threads, control, msg);
      continue;
    }

    if (TQMESSAGE_WCOMBINATOR == msg->flag) {
      // a worker who will generate words of given N length, and queue a
      // message to try match any of the words.. etc
      // tqueue_worker_wcombinator(threads, control, msg);
      // tqueue_send(control, msg, TQMESSAGE_WCOMBINATOR, NULL, 1);
      continue;
    }

    if (TQMESSAGE_CLOSE == msg->flag) {
      if (--msg->argn > 0)
        tqueue_send(threads, msg, TQMESSAGE_CLOSE, NULL, msg->argn, 0);
      break;
    }

    break;
  }
  pthread_cleanup_pop(1);
}

void tqueue_worker_root_cleanup(void* arg) {
  struct tqueue_message msg = { 0 };
  tqueue_send(*(mqd_t*) arg, &msg, TQMESSAGE_POP, NULL, 0, 1);
}

void tqueue_worker_word_tester(mqd_t threads, mqd_t control, struct tqueue_message* msg) {
  // char* crypt_result;
  struct crypt_data crypt_data;
	crypt_data.initialized = 0;
  int length;
  do {
    if (TQMESSAGE_TEST_WORDS != msg->flag) break;
    // We now take the responsebility to free the allocated memory in the message
    // in this case its the test_word struct and a char* buffer
    pthread_cleanup_push(tqueue_worker_word_tester_cleanup, msg->arg);
    struct test_words* words = msg->arg;
    length = words->length;
    for (int i = 0; i < words->length; i += 1 + strlen(words->buffer + i)) {
      char* crypt_result = crypt_r(words->buffer + i, client_config.salt, &crypt_data);
      if (strncmp(crypt_result, client_config.hash, 34) == 0) {
        // now... in theory it's shouldnt be needed to block cancel of
        // the thread, since we only cancel threads when we found a password, and
        // this is that message, is it impossible for two threads to find a password
        // at the same time? Well, lets be safe eh? its graded.
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        int password_length = strlen(words->buffer + i);
        char* password = calloc(password_length + 1, sizeof(char));
        memcpy(password, words->buffer + i, password_length);
        tqueue_send(control, msg, TQMESSAGE_PASSWORD, password, 0, 5);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        // its in main threads hand to free password
        break;
      }
    }
    // Cleanup the test_word memory on the heap
    pthread_cleanup_pop(1);
    // Send a message with the total length of words processed to main, so it
    // cant update the progress indicator in terminal output
    tqueue_send(control, msg, TQMESSAGE_TEST_WORDS, NULL, length, 5);
  } while (tqueue_read(threads, msg));
}

void tqueue_worker_word_tester_cleanup(void* arg) {
  struct test_words* words = arg;
  free(words->buffer);
  free(words);
}

/*
  It will create a buffer of fixed size (can be changed via terminal args)
  and put them on a messaging queue (mqueue), where other threads can pick
  them off the queue and process them.

  The buffer is allocated on the heap, so when the 'message' with the buffer
  has been sent, this function is no longer responsible to dealloc it,
  its the responsebility of the other threads picking the message up.
*/
void tqueue_worker_wdictionary(mqd_t threads, mqd_t control, struct tqueue_message* msg) {
  (void)(control); // Well, this is annoying, learned in class to fix all warnings... =P
  long wdict_sizeish;
  struct wdictionary wdict;
  if (wdictionary_init(&wdict, (char*)msg->arg, &wdict_sizeish) != 0) {
    // TDD(lennart): handle errors plox
    // TDD(lennart): maybe research if we can reroute the stdout to a buffer, and send that
    // to the main thread, in an error signal.. of sorts.
    // printf("WDICTERROR\n");
  }
  pthread_cleanup_push(&tqueue_worker_wdictionary_cleanup, &wdict);
  // Send a message to main thread about the 'expected' size of file, so we can calculate
  // an estimate time left
  tqueue_send(control, msg, TQMESSAGE_WDICTIONARY, &wdict_sizeish, 0, 1);

  do {
    // We block cancellation of thread until we can release responsebility of the buffer
    // and test_word on heap
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    char* buffer = malloc(client_config.thread_buffer_size * sizeof(char));
    int length = wdictionary_fill(&wdict, buffer, client_config.thread_buffer_size);
    if (length > 0) {
      struct test_words* words = malloc(sizeof(struct test_words));
      words->buffer = buffer;
      words->length = length;
      tqueue_send(threads, msg, TQMESSAGE_TEST_WORDS, words, 0, 1);
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      pthread_testcancel();
      continue;
    }
    // There wasnt anymore words from the wdictionary file, so were done.
    free(buffer);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();
    break;
  } while(1);
  // Send a message to the control that the dictionary is done, control
  // will issue a new message appropriately to read anoter file, or try
  // generating words based on used input
  // Pass NULL instead of a long* as earlier, indicating we are with
  // with this dictionary
  tqueue_send(control, msg, TQMESSAGE_WDICTIONARY, NULL, 0, 1);
  pthread_cleanup_pop(1);
}

void tqueue_worker_wdictionary_cleanup(void* arg) {
  wdictionary_free((struct wdictionary*) arg);
}

void tqueue_worker_flush(mqd_t threads, mqd_t control, struct tqueue_message* msg) {
  (void)(control);
  struct timespec tm;
  clock_gettime(CLOCK_REALTIME, &tm);
  tm.tv_nsec += 100000;
  while (mq_timedreceive(threads, (char*) msg, sizeof(struct tqueue_message), NULL, &tm)) {
    if (TQMESSAGE_TEST_WORDS == msg->flag) {
      tqueue_worker_word_tester_cleanup(msg->arg);
      break;
    }
    if (TQMESSAGE_WDICTIONARY == msg->flag) continue;
    if (TQMESSAGE_WCOMBINATOR == msg->flag) continue;
    break;
  }
}

void tqueue_worker_close(mqd_t threads, mqd_t control, struct tqueue_message* msg) {
  (void)(control);
  struct mq_attr attr;
  while (1) {
    mq_getattr(threads, &attr);
    if (attr.mq_curmsgs == 0) {
      for (int i = 0; i < client_config.thread_count; i++)
        tqueue_send(threads, msg, TQMESSAGE_CLOSE, NULL, i, 0);
      break;
    }
    usleep(10000);
  }
}