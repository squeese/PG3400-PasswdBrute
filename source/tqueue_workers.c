#define _GNU_SOURCE 1
#include "tqueue_workers.h"
#include "wcombinator.h"
#include "vmap.h"
#include <stdlib.h>
#include <string.h>
#include <crypt.h>

void tqueue_worker_root_cleanup(void* arg) {
  struct tqueue_message msg = { 0 };
  tqueue_send(*(mqd_t*) arg, &msg, TQMESSAGE_POP, NULL, 0, 1);
}

void tqueue_worker_root(mqd_t threads, mqd_t control, struct tqueue_message* msg) {
  // pthread_detach(pthread_self());
  // Send message to 'control' that were running, and register a cleanup
  // function to send a message that were done.
  tqueue_send(control, msg, TQMESSAGE_PUSH, NULL, 0, 3);
  pthread_cleanup_push(&tqueue_worker_root_cleanup, &control);

  // Read from the theads channel and process
  while (tqueue_read(threads, msg)) {

    if ((TQMESSAGE_TEST_FILE_WORDS | TQMESSAGE_TEST_GENERATED_WORDS) & msg->flag) {
      // the message has a buffer if N size (defined by user terminal input)
      // that contains words, separated by \0 terminator.
      // we 'defer control' of the thread to this worker, it will read messages
      // until it finds one it wont handle
      if (TQMESSAGE_TEST_FILE_WORDS == msg->flag)
        tqueue_worker_word_file_tester(threads, control, msg);
      else
        tqueue_worker_word_generated_tester(threads, control, msg);
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

    if (TQMESSAGE_FLUSH == msg->flag) {
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
      tqueue_send(control, msg, TQMESSAGE_FLUSH, NULL, 0, 10);
      while (tqueue_read(threads, msg)) {
        // When we get the next flush message, we cancel the loop, and unblock
        // cancellation of the thread, thus this thread also exits
        if (TQMESSAGE_FLUSH == msg->flag) break;
        if (TQMESSAGE_TEST_FILE_WORDS == msg->flag)
          tqueue_worker_word_file_tester_cleanup(msg->arg);
        if (TQMESSAGE_TEST_GENERATED_WORDS == msg->flag)
          tqueue_worker_word_generated_tester_cleanup(msg->arg);
      }
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      pthread_testcancel();
      break;
    }

    if (TQMESSAGE_CLOSE == msg->flag) {
      // The message signals a shutdown of the threads, only one message is sent
      // initially by the main thread, but we propagate the message back into the
      // threads queue, until we sent one for each thread.
      // It's done this way, instead of main thread (controller) sending all the
      // messages because that would be an instant deadlock.
      if (--msg->argn > 0)
        tqueue_send(threads, msg, TQMESSAGE_CLOSE, NULL, msg->argn, 0);
      break;
    }

    break;
  }
  pthread_cleanup_pop(1);
}

void tqueue_worker_word_file_tester_cleanup(void* arg) {
  free((char**) arg);
}

void tqueue_worker_word_file_tester(mqd_t threads, mqd_t control, struct tqueue_message* msg) {
  // char* crypt_result;
  struct crypt_data crypt_data;
	crypt_data.initialized = 0;
  int length;
  do {
    if (TQMESSAGE_TEST_FILE_WORDS != msg->flag) break;
    // We now take the responsebility to free the allocated memory in the message
    pthread_cleanup_push(tqueue_worker_word_file_tester_cleanup, msg->arg);
    char** indices = msg->arg;
    length = *(indices + msg->argn - 1) - *indices + strlen(*(indices + msg->argn - 1));
    for (int i = 0; i < msg->argn; i++) {
      char* crypt_result = crypt_r(*(indices + i), client_config.salt, &crypt_data);
      if (strncmp(crypt_result, client_config.hash, 34) == 0) {
        // now... in theory it's shouldnt be needed to block cancel of
        // the thread, since we only cancel threads when we found a password, and
        // this is that message, is it impossible for two threads to find a password
        // at the same time? Well, lets be safe eh? its graded.
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        int length = strlen(*(indices + i));
        char* password = calloc(length + 1, sizeof(char));
        memcpy(password, *(indices + i), length);
        tqueue_send(control, msg, TQMESSAGE_PASSWORD, password, 0, 5);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
        // It is the main thread's responsebility to deallocate the password
        break;
      }
    }
    pthread_cleanup_pop(1);
    tqueue_send(control, msg, TQMESSAGE_TEST_FILE_WORDS, NULL, length, 5);
  } while (tqueue_read(threads, msg));
}

void tqueue_worker_word_generated_tester_cleanup(void* arg) {
  free((char*) arg);
}

void tqueue_worker_word_generated_tester(mqd_t threads, mqd_t control, struct tqueue_message* msg) {
  // char* crypt_result;
  struct crypt_data crypt_data;
	crypt_data.initialized = 0;
  do {
    if (TQMESSAGE_TEST_GENERATED_WORDS != msg->flag) break;
    // We now take the responsebility to free the allocated memory in the message,
    // in this case its the char* buffer
    pthread_cleanup_push(tqueue_worker_word_generated_tester_cleanup, msg->arg);
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
    tqueue_send(control, msg, TQMESSAGE_TEST_GENERATED_WORDS, NULL, msg->argn, 5);
  } while (tqueue_read(threads, msg));
}


// void tqueue_worker_wdictionary_cleanup(void* arg) {}

void tqueue_worker_wdictionary(mqd_t threads, mqd_t control, struct tqueue_message* msg) {
  (void)(control);
  char* cursor = ((struct vmap*) msg->arg)->address;
  int total_pending = 0;
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
      for (; *cursor == '\n' || *cursor == '\r'; cursor++);
    }
    // Issue a message to start threads 
    total_pending++;
    tqueue_send(threads, msg, TQMESSAGE_TEST_FILE_WORDS, indices, index, 1);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();
    // Issue a message to the control thread (main), that there are N amount
    // of words that has been queued for testing, the main thread need to
    // keep track of whats left to test in the pagefile, before we can swap it
    // to the next file.
  } while(*cursor != 0);
  tqueue_send(control, msg, TQMESSAGE_WDICTIONARY, NULL, total_pending, 1);
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
      tqueue_send(threads, msg, TQMESSAGE_TEST_GENERATED_WORDS, buffer, length, 1);
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
  tqueue_send(control, msg, TQMESSAGE_WCOMBINATOR, NULL, 100, 1);
}

void tqueue_worker_wcombinator_cleanup(void* arg) {
  wcomb_free((struct wcombinator*) arg);
}