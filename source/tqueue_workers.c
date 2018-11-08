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
  tqueue_send(control, msg, TQM_PUSH, NULL, 0, 3);
  pthread_cleanup_push(&tqueue_worker_root_cleanup, &control);

  // Read from the theads channel and process
  while (tqueue_read(threads, msg)) {

    if (TQM_PAGEFILE_CHUNK == msg->flag) {
      // the message has a buffer if N size (defined by user terminal input)
      // that contains words, separated by \0 terminator.
      // we 'defer control' of the thread to this worker, it will read messages
      // until it finds one it wont handle
      tqueue_worker_word_tester(threads, control, msg);
      // we dont issue continue on the while loop here and read a new message,
      // we just let it fall through; since word_tester fn exits when it reads
      // a non TQM_PAGEFILE_CHUNK message, so someone else needs to handle it
    }

    if (TQM_RETURN & msg->flag) {
      // return flag indicates that control thread (main) wants a flag
      // sent to itself, its a work around for deadlock.
      // So we mask away the return flag from the original
      tqueue_send(control, msg, msg->flag & ~TQM_RETURN, msg->arg, msg->argn, 1);
      continue;
    }

    if (TQM_WDICTIONARY == msg->flag) {
      // a worker who reads words from a file, and queues a message to try
      // and match any of the words against the md5hash we got, to try find
      // password
      tqueue_worker_wdictionary(threads, control, msg);
      continue;
    }

    if (TQM_WCOMBINATOR == msg->flag) {
      // a worker who will generate words of given N length, and queue a
      // message to try match any of the words.. etc
      tqueue_worker_wcombinator(threads, control, msg);
      continue;
    }

    if (TQM_FLUSH == msg->flag) {
      // The main thread has found a password, and is shutting down the threads,
      // its doing this by calling pthread_cancel, but first it will send one of
      // the threads into this state; blocking the pthread_cancel, read and dealloc
      // messages in the queue until it receives another message to stop.
      // This is done to prevent deadlock during the cancellation of threads, since
      // wcombinator_worker and wdictionary_worker can be stuck trying to send
      // a message with allocated memory (blocking cancellation aswell), so some
      // thread must be reading those messages off the queue, and dealloc
      pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
      // Send a message back to the control (main thread) that we are 'flushing'
      // allocated messages
      tqueue_send(control, msg, TQM_FLUSH, NULL, 0, 10);
      while (tqueue_read(threads, msg)) {
        // When we get the next flush message, we cancel the loop, and unblock
        // cancellation of the thread, thus this thread also exits
        if (TQM_FLUSH == msg->flag) break;
        if (TQM_PAGEFILE_CHUNK == msg->flag)
          tqueue_worker_word_tester_cleanup(msg->arg);
      }
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      pthread_testcancel();
      break;
    }

    if (TQM_CLOSE == msg->flag) {
      // The message signals a shutdown of the threads, only one message is sent
      // initially by the main thread, but we propagate the message back into the
      // threads queue, until we sent one for each thread.
      // It's done this way, instead of main thread (controller) sending all the
      // messages because that would be an instant deadlock.
      if (--msg->argn > 0)
        tqueue_send(threads, msg, TQM_CLOSE, NULL, msg->argn, 0);
      break;
    }

    break;
  }
  pthread_cleanup_pop(1);
}

void tqueue_worker_root_cleanup(void* arg) {
  struct tqueue_message msg = { 0 };
  tqueue_send(*(mqd_t*) arg, &msg, TQM_POP, NULL, 0, 1);
}

void tqueue_worker_wdictionary(mqd_t threads, mqd_t control, struct tqueue_message* msg) {
  (void)(control); // Well, this is annoying, learned in class to fix all warnings... =P
  char* cursor = (char*) msg->arg;
  do {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    char** indices = malloc(client_config.thread_buffer_size * sizeof(char*));
    int index = 0;
    for (; index < client_config.thread_buffer_size && *cursor != 0; index++) {
      indices[index] = cursor;
      // scan past all word characters to the next linebreak
      for (; *cursor != '\n' && *cursor != '\r'; cursor++)
        if (*cursor == 0) break;
      *cursor = 0;  // make sure we \0 terminate after each word
      cursor++;     // move the cursor past the newline characters
      for (; *c == '\n' || *c == '\r'; c++);
    }
    // Issue a message to start threads 
    tqueue_send(threads, msg, TQM_TEST_WORDS, indices, i, 1);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();
    // Issue a message to the control thread (main), that there are N amount
    // of words that has been queued for testing, the main thread need to
    // keep track of whats left to test in the pagefile, before we can swap it
    // to the next file.
  } while(*c != 0);
  tqueue_send(control, msg, TQM_WDICTIONARY, NULL, total_chunks_pending, 1);
}

// pthread_cleanup_push(&tqueue_worker_wdictionary_cleanup, &wdict);
// unsigned long size = sb.st_size;
// tqueue_send(control, msg, TQM_WDICTIONARY, &size, 0, 1);
void tqueue_worker_wdictionary_cleanup(void* arg) {
  wdictionary_free((struct wdictionary*) arg);
}

void tqueue_worker_word_tester(mqd_t threads, mqd_t control, struct tqueue_message* msg) {
  struct crypt_data crypt_data;
	crypt_data.initialized = 0;
  int chunk_length;
  do {
    if (TQM_PAGEFILE_CHUNK != msg->flag) break;
    // We now take the responsebility to free the allocated memory in the message,
    // in this case its the char* buffer
    pthread_cleanup_push(tqueue_worker_word_tester_cleanup, msg->arg);
    char** indices = msg->arg;
    int length = msg->argn;

     = *(indices + length - 1) - *indices + strlen(*(indices + length - 1));

    weight = (pagefile_word_ptrs[length - 1] - pagefile_word_ptrs) + strlen(pagefile_word_ptrs)

    for (int index = 0; index < length; index++) {
      char* crypt_result = crypt_r(pagefile_word_ptrs[index], client_config.salt, &crypt_data);
      if (strncmp(crypt_result, client_config.hash, 34) == 0) {
        // since we are passing on the re.. er
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        int length = strlen(pagefile_word_ptrs[index]);
        char* password = calloc(length + 1, sizeof(char));
        memcpy(password, pagefile_word_ptrs[index], length);
        tqueue_send(control, msg, TQM_PASSWORD, password, length, 5);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        // its in main threads hands to free password
        break;
      }
    }
    // Cleanup the test_word memory on the heap
    pthread_cleanup_pop(1);
    // Send a message with the total length of words processed to main, so it
    // cant update the progress indicator in terminal output
    tqueue_send(control, msg, TQM_TEST_DONE, NULL, msg->argn, 5);
  } while (tqueue_read(threads, msg));
}

void tqueue_worker_word_tester_cleanup(void* arg) {
  free((char**) arg);
}

void tqueue_worker_wcombinator(mqd_t threads, mqd_t control, struct tqueue_message* msg) {
  (void)(control);
  /*
  struct wcombinator wcomb;
  long num_combinations = wcomb_init(&wcomb, msg->argn, client_config.input_buffer, client_config.input_length);
  pthread_cleanup_push(&tqueue_worker_wcombinator_cleanup, &wcomb);
  // Send a message to the control with the total length (sum) of all words that will
  // be generated, will be used to update the progress indicator
  tqueue_send(control, msg, TQM_WCOMBINATOR, &num_combinations, 0, 1);
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
      if (tqueue_send(threads, msg, TQM_PAGEFILE_CHUNK, buffer, length, 1) >= 0) {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        continue;
      }
      // resolve deadlock
      printf("WTF\n");
    }
    free(buffer);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();
    break;
  }
  pthread_cleanup_pop(1);
  // Send a message to the control that were done
  tqueue_send(control, msg, TQM_WCOMBINATOR, NULL, 0, 1);
  */
}

void tqueue_worker_wcombinator_cleanup(void* arg) {
  wcomb_free((struct wcombinator*) arg);
}