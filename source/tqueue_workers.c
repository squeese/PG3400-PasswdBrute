#define _GNU_SOURCE 1
#include "tqueue_workers.h"
#include "wdictionary.h"
#include "wcombinator.h"
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
      tqueue_worker_wcombinator(threads, control, msg);
      continue;
    }

    if (TQMESSAGE_CLOSE == msg->flag) {
      if (--msg->argn > 0) {
        tqueue_send(threads, msg, TQMESSAGE_CLOSE, NULL, msg->argn, 0);
      }
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
  do {
    if (TQMESSAGE_TEST_WORDS != msg->flag) break;
    // We now take the responsebility to free the allocated memory in the message,
    // in this case its the char* buffer
    pthread_cleanup_push(tqueue_worker_word_tester_cleanup, msg->arg);
    char* buffer = (char*) msg->arg;
    for (int i = 0; i < msg->argn; i += 1 + strlen(buffer + i)) {
      char* crypt_result = crypt_r(buffer + i, client_config.salt, &crypt_data);
      if (strncmp(crypt_result, client_config.hash, 34) == 0) {
        // now... in theory it's shouldnt be needed to block cancel of
        // the thread, since we only cancel threads when we found a password, and
        // this is that message, is it impossible for two threads to find a password
        // at the same time? Well, lets be safe eh? its graded.
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        int password_length = strlen(buffer + i);
        char* password = calloc(password_length + 1, sizeof(char));
        memcpy(password, buffer + i, password_length);
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
    tqueue_send(control, msg, TQMESSAGE_TEST_WORDS, NULL, msg->argn, 5);
  } while (tqueue_read(threads, msg));
}

void tqueue_worker_word_tester_cleanup(void* arg) {
  free((char*) arg); // buffer
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
      tqueue_send(threads, msg, TQMESSAGE_TEST_WORDS, buffer, length, 1);
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
  pthread_cleanup_pop(1);
  // Send a message to the control that the dictionary is done, control
  // will issue a new message appropriately to read anoter file, or try
  // generating words based on used input
  // Pass NULL instead of a long* as earlier, indicating we are with
  // with this dictionary
  tqueue_send(control, msg, TQMESSAGE_WDICTIONARY, NULL, 0, 1);
}

void tqueue_worker_wdictionary_cleanup(void* arg) {
  wdictionary_free((struct wdictionary*) arg);
}

void tqueue_worker_wcombinator(mqd_t threads, mqd_t control, struct tqueue_message* msg) {
  (void)(control);
  struct wcombinator wcomb;
  long num_combinations = wcomb_init(&wcomb, msg->argn, client_config.input_buffer, client_config.input_length);
  pthread_cleanup_push(&tqueue_worker_wcombinator_cleanup, &wcomb);
  // Send a message to the control with the total length (sum) of all words that will
  // be generated, will be used to update the progress indicator
  tqueue_send(control, msg, TQMESSAGE_WCOMBINATOR, &num_combinations, 0, 1);
  // need to know how many words we can fit into the given buffer size
  // wcombinator's generate is based on how many words you want generated,
  // dont see the need to refactor it to conform to wdictionary_fill, that will
  // just generate words, until the given buffer is full
  int word_count = client_config.thread_buffer_size / (sizeof(char) * (wcomb.word_size + 1));
  int buffer_size = client_config.thread_buffer_size * sizeof(char);
  while (1) {
    // Blocking the pthread_cancel calls untils we can release the responsebility
    // to deallocate the buffer, or deallocate it ourselfs
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    char* buffer = malloc(buffer_size);
    int length;
    if ((length = wcomb_generate(&wcomb, buffer, word_count)) > 0) {
      tqueue_send(threads, msg, TQMESSAGE_TEST_WORDS, buffer, length, 1);
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      pthread_testcancel();
      continue;
    }
    free(buffer);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();
    break;
  }
  pthread_cleanup_pop(1);
  // Send a message to the control that were done
  tqueue_send(control, msg, TQMESSAGE_WCOMBINATOR, NULL, 0, 1);
}

void tqueue_worker_wcombinator_cleanup(void* arg) {
  wcomb_free((struct wcombinator*) arg);
}