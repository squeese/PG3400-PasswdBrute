#include "args.h"
#include "tqueue.h"
#include "tqueue_workers.h"
#include "progress.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

struct args_client_config client_config;

int main(int argc, char** args) {
  // Process the arguments and create a config
  if (args_client_init(&client_config, argc, args) != 0) {
    args_client_free(&client_config);
    return EXIT_FAILURE;
  }

  // Dump the current configs passed to the console
  printf("> HASH     : %s\n", client_config.hash);
  printf("> INPUTS   : length:%d, %s\n", client_config.input_length, client_config.input_buffer);
  printf("> LENGTH   : start:%d, end:%d\n", client_config.word_length_min, client_config.word_length_max);
  printf("> THREAD   : count:%d, size:%d\n", client_config.thread_count, client_config.thread_buffer_size);

  // Create and initialize a tqueue instance
  struct tqueue tq;
  if (tqueue_init(&tq) != 0) {
    args_client_free(&client_config);
    return EXIT_FAILURE;
  }

  // Spawn pthreads with the default tqueue_thread_worker
  // We use tqueue to spawn the instances, but we get the pthread_t instance
  // in return, we keep those, so we can cancel the threads if we find a password
  pthread_t* worker_threads = malloc(client_config.thread_count * sizeof(pthread_t));
  for (int i = 0; i < client_config.thread_count; i++)
    tqueue_run(&tq, worker_threads + i, &tqueue_worker_root);

  // A simple terminal widget to help visualize progress on the current task
  // It gives a rough estimate, not accurate =)
  struct progress prog;
  char* password = NULL;
  // When exiting the application (we found a password) and there is still messages
  // queued, and/or threads are currently working, the main thread cant quit until
  // all threads have completed their jobs, and deallocated resources they are using
  // So, when threads are done, they send a message, we keep track of that.
  int threads_active = 0;
  int index_dictionary = 0;
  int index_combiner = client_config.word_length_min;

  // This is the 'controller'.
  // The threads spawned above send messages to the tqueue channel 'controller' with
  // results of their labor, read, process, send back more messages with instructions
  // for more jobs, etc.
  struct tqueue_message msg = { 0 };
  while (tqueue_read(tq.queue_control, &msg)) {

    if (TQMESSAGE_PUSH == msg.flag) {
      if (threads_active++ == 0) {
        // First thread worker has started, we can now issue a worker to start procude
        // words to test agains the hash.
        // The TQMESSAGE_RETURN sent to the treads queue, is indicating I want it sent back
        // to this thread (main). This is because writing directly to the control queue,
        // there is a good change to end up in deadlock, since the main thread is the only
        // one reading from control
        tqueue_send(tq.queue_threads, &msg, (TQMESSAGE_RETURN | TQMESSAGE_NEXT), NULL, 0, 1);
      }
      continue;
    }

    if (TQMESSAGE_NEXT == msg.flag) {

      // First in priority for searching for password is a dictionary file, we will create
      // a wdictionary_worker for each dictionary specified in the -d path.txt file
      if (index_dictionary < client_config.dictionary_count) {
        char* path = *(client_config.dictionary_paths + index_dictionary++);
        // Reset the progress indicator to show the dictionary
        progress_init(&prog);
        progress_title(&prog, snprintf(prog.title, 64, "Dictionary: %s", path));
        // Issue a message to the threads to spawn a wdictionary_worker
        tqueue_send(tq.queue_threads, &msg, TQMESSAGE_WDICTIONARY, path, 0, 1);

      // Next up is a word combiner to generate words
      // We must spawn a wcombiner_worker for each length of word user wants to try
      // So if specified to search for words up to 5 characters, we spawn 5 wcombinator_workers
      // in sequence
      } else if (index_combiner <= client_config.word_length_max) {
        // Reset the progress indicator to show the dictionary
        progress_init(&prog);
        progress_title(&prog, snprintf(prog.title, 64, "Word Size %d, Search Area %d, Solutions %d", index_combiner, client_config.input_length, 0));
        // Issue a message to the threads to spawn a wcombiner_worker
        tqueue_send(tq.queue_threads, &msg, TQMESSAGE_WCOMBINATOR, NULL, index_combiner++, 1);

      // No more tasks that will generate words.
      // Issue a signal to begin closing down the threads and exit
      } else {
        tqueue_send(tq.queue_threads, &msg, (TQMESSAGE_RETURN | TQMESSAGE_CLOSE), NULL, 0, 1);
      }
      continue;
    }

    if ((TQMESSAGE_WDICTIONARY | TQMESSAGE_WCOMBINATOR) & msg.flag) {
      // wdictionary_worker or wcombinator_worker has sent a message that it's started processing
      // msg.arg contains the 'approximate' length of all words it will be producing
      if (msg.arg != NULL) {
        prog.max = *(long*) msg.arg;
        if (TQMESSAGE_WCOMBINATOR == msg.flag) {
          // update the 'combinations' in the title of progress bar
          progress_title(&prog, snprintf(prog.title, 64, "Word Size %d, Search Area %d, Combinations %ld", index_combiner - 1, client_config.input_length, *(long*) msg.arg)); 
        }

      // This time its a message it's done processing the dictionary file
      } else {
        // This 'freezes' the progress indicator, since its really just guestimation on the
        // progress, the numbers are really off. So its zeroed. Looks better.
        progress_finish(&prog);
        // Issue a message to queue another word generator worker
        tqueue_send(tq.queue_threads, &msg, (TQMESSAGE_RETURN | TQMESSAGE_NEXT), NULL, 0, 1);
      }
      continue;
    }

    if (TQMESSAGE_TEST_WORDS == msg.flag) {
      // worder_tester_worker has finished processing a batch of words, only thing
      // we do is update the progress indicator
      progress_update(&prog, msg.argn);
      continue;
    }

    if (TQMESSAGE_PASSWORD == msg.flag) {
      // Good stuff, keep the password for later, and issue a message to begin
      // the process of closing down
      password = (char*) msg.arg;
      tqueue_send(tq.queue_threads, &msg, (TQMESSAGE_RETURN | TQMESSAGE_PASSWORD | TQMESSAGE_CLOSE), NULL, 0, 1);
      continue;
    }

    // Begin the process of closing down the threads properly and clean
    // up after ourselfs. There are two different ways we end up here,
    // found a password and we didnt; we have to handle them differently.
    // *note: 90% of memory leaks stemmed from here, all them sneaky little
    // hobbitses stealing ramsies
    if (TQMESSAGE_CLOSE & msg.flag) {
      if (TQMESSAGE_PASSWORD & msg.flag) {
        // In the case we found a password, we use pthread_cancel to forcefully
        // close the threads, since they are propably busy processing stuff, and
        // there are stuff queued up, its unreliable to send close message to the
        // threads. The dictionary worker might be reading a huge file, not good.
        // So we issue pthread_cancel
        for (int i = 0; i < client_config.thread_count; i++)
          pthread_cancel(*(worker_threads + i));
        // But that may leave messages queued up in the channels that wont be processes
        // by the threads; which would be a disaster for memory leaks. The messages
        // reference large swathes of data in heap. So we have to flush the queues
        // before we exit the application.
      } else {
        // The other case, is we finished all the tasks that produce words, and we
        // still havent found any matches. Here we cant really close the threads
        // directly, since there might still be a valid password close to be found
        // in another thread, or queued up.
        // Here we just issue TQMESSAGE_CLOSE with the lowest priority, one for
        // each thread (they will only pick one up each, since its the last one they
        // can read), To avoid deadlock and flooding the threads queue by sending all
        // messages at once; we send them one by one by letting the threads resend it
        // by decrementing a counter
        tqueue_send(tq.queue_threads, &msg, TQMESSAGE_CLOSE, NULL, client_config.thread_count, 0);
        // *note: turns out this wasnt that complicated after all, but I guess it's
        // always that way, once you get a good enough solution.
      }
      continue;
    }

    if (TQMESSAGE_POP == msg.flag) {
      // When the last child thread has exited, there isnt anyone that will be sending
      // messages to the control queue, safe to quit
      if (--threads_active == 0) break;
      continue;
    }
  }

  // Join all child threads, free up the memory associated with the threads
  for (int i = 0; i < client_config.thread_count; i++) {
    pthread_join(*(worker_threads + i), NULL);
  }

  // Check if there are any messages queued still in the message queues, that
  // has any memory allocations we need to free before exiting. This can happen
  // when we find a password, while still having data queued up for processing
  // Alter the message queues to O_NONBLOCK, so we can read from the channels
  // until its empty, and return a EAGAIN code when its empty, istead of blocking
  tqueue_unblock(&tq);
  // *note: after some refactoring, got it down to only one message that could
  // leak at this point, good stuff
  while (tqueue_read(tq.queue_control, &msg) > 0);
  while (tqueue_read(tq.queue_threads, &msg) > 0) {
    if (TQMESSAGE_TEST_WORDS == msg.flag) {
      // buffer from wcombiner_worker and wdictionary_worker
      free((char*) msg.arg);
    }
  }


  // Write to the console the result
  if (password != NULL) {
    printf("\n\n> PASSWORD : %s\n\n", password);
    free(password);
  } else {
    printf("\n\n> PASSWORD : NO MATCH\n\n");
  }

  // Cleanup
  free(worker_threads);
  args_client_free(&client_config);
  tqueue_free(&tq);

  // Only way to get valgrind not to be upset about open file descriptors
  // with the flag --track-fds=yes
  fclose(stdin);
  fclose(stdout);
  fclose(stderr);
  return EXIT_SUCCESS;
}