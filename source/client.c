#include "args.h"
#include "tqueue.h"
#include "tqueue_workers.h"
#include "progress.h"
#include "vmap.h"
#include <stdio.h>
#include <stdlib.h>

struct args_client_config client_config;

int main(int argc, char** args) {
  // Process the arguments and create a config
  if (args_client_init(&client_config, argc, args) != 0) {
    args_client_free(&client_config);
    return EXIT_FAILURE;
  }

  // Dump the current configs passed to the console
  printf("\n> HASH     : %s\n", client_config.hash);
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
  struct vmap vmap;
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
      // Signal that indicates a thread has started
      if (threads_active++ == 0) {
        // Issue a signal to create a thread worker which will produce words to test against
        // the hash.
        tqueue_send(tq.queue_threads, &msg, (TQMESSAGE_RETURN | TQMESSAGE_NEXT), NULL, 0, 1);
        // The message is meant for this thread (main), but will be sent to the threads queue.
        // Nifty little trick where we add a flag to indicate we want it sent back; this way
        // we work around the problem of deadlock when writing to a channel you also have
        // the responsebility to make sure messages are popped off.
      }
      continue;
    }

    if (TQMESSAGE_NEXT == msg.flag) {

      if (index_dictionary < client_config.dictionary_count) {
        // First in priority for searching for password is a dictionary file.
        // Spawn one for each -d <pathname> flag used when running application
        char* path = *(client_config.dictionary_paths + index_dictionary++);
        vmap_load_file(&vmap, path);
        progress_init(&prog, vmap.size);
        progress_title(&prog, snprintf(prog.title, 64, "Dictionary: %s", path));
        tqueue_send(tq.queue_threads, &msg, TQMESSAGE_WDICTIONARY, &vmap, 0, 1);

      } else if (index_combiner <= client_config.word_length_max) {
        // Next up is a word combiner to generate words
        // We must spawn a wcombiner_worker for each length of word user wants to try
        // Users can use either -l <num> to indicate length of word to search for
        // or -L <n-m> which indicates a range from N to M, eks: -L 1-3
        progress_init(&prog, 1000000);
        progress_title(&prog, snprintf(prog.title, 64, "Word Size %d, Search Area %d, Solutions %d", index_combiner, client_config.input_length, 0));
        tqueue_send(tq.queue_threads, &msg, TQMESSAGE_WCOMBINATOR, NULL, index_combiner++, 1);

      } else {
        // No more workers to start that will produce words to test against
        // Issue a signal to begin closing down the threads and exit
        tqueue_send(tq.queue_threads, &msg, (TQMESSAGE_RETURN | TQMESSAGE_CLOSE), NULL, 0, 1);
      }
      continue;
    }

    if ((TQMESSAGE_WDICTIONARY | TQMESSAGE_WCOMBINATOR) & msg.flag) {
      // wdictionary_worker or wcombinator_worker has sent a message that it's started processing.
      // msg.arg contains the 'approximate' length of all words it will be producing
      if (msg.arg != NULL) {
        prog.max = *(long*) msg.arg;
        if (TQMESSAGE_WCOMBINATOR == msg.flag) {
          // update the 'combinations' in the title of progress bar
          progress_title(&prog, snprintf(prog.title, 64, "Word Size %d, Search Area %d, Combinations %ld", index_combiner - 1, client_config.input_length, *(long*) msg.arg)); 
        }

      // Or when they are finished producing words
      } else {
        // This 'freezes' the progress indicator, since its really just guestimation on the
        // progress, the numbers are really off. So its zeroed. Looks better.
        progress_finish(&prog);
        printf("\n");
        // Issue a message to queue another word generator worker
        tqueue_send(tq.queue_threads, &msg, (TQMESSAGE_RETURN | TQMESSAGE_NEXT), NULL, 0, 1);
      }
      continue;
    }

    if ((TQMESSAGE_TEST_GENERATED_WORDS | TQMESSAGE_TEST_FILE_WORDS) & msg.flag) {
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
    // up all allocations made in the thread.
    // There are two different ways we end up here,
    // found a password and we didnt; we have to handle them differently.
    if (TQMESSAGE_CLOSE & msg.flag) {
      if (TQMESSAGE_PASSWORD & msg.flag) {
        // In the case we found a password, we use pthread_cancel to forcefully
        // close the threads, since some threads are busy producing messages for
        // other threads, they dont actually read messages from the queue. That
        // being; wdictionary_worker and wcombiner_worker.
        // So to initiate the shutdown directly with pthread_cancel on each thread.
        // However, threads can be in state where they block cancellation, when
        // they are waiting to write to the queue (when its full), in order to
        // protect against memory leaks. That problem would only occure if the
        // queue is full, we can solve that by making sure 'someone' is always
        // reading messages of the queue.
        // That is done by issuing a TQMESSAGE_FLUSH to the threads, one thread
        // will go into cancellation block and read messages, and free up the
        // allocated memory.
        tqueue_send(tq.queue_threads, &msg, TQMESSAGE_FLUSH, NULL, 0, 10);
        // When control (main thread) recieves a TQMESSAGE_FLUSH signal, the
        // cancellation can occur.
      } else {
        // The other case is when wdictionary_worker and wcombiner_worker are done
        // producing words, and we still havent found any matches from the data
        // so faar processed. But we cant outright cancel the threads though, since
        // there might be data left in processing or queued. So we handle this by
        // issuing a TQMESSAGE_CLOSE signal to the threads, with the lowest priority.
        // We can guarantee that this message will be handled by all threads, since
        // they must all be in a state listening for messages now.
        // We only send one message though, sending one for each thread risk's deadlock.
        // Send one, with a value indicating how many we want sent, the threads propagate
        // the message among themselfs.
        tqueue_send(tq.queue_threads, &msg, TQMESSAGE_CLOSE, NULL, client_config.thread_count, 0);
      }
      continue;
    }

    if (TQMESSAGE_FLUSH == msg.flag) {
      // As explained above, one thread is now blocking cancellation, and reading from
      // the threads channel, to ensure no deadlock, and all messages containing allocated
      // memory is properly freed.
      for (int i = 0; i < client_config.thread_count; i++)
        pthread_cancel(*(worker_threads + i));
      // Issue a new message to the blocking thread, that it can now cancel the block
      // and terminate.
      tqueue_send(tq.queue_threads, &msg, TQMESSAGE_FLUSH, NULL, 0, 0);
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
  // until its empty, whitout blocking.
  tqueue_unblock(&tq);
  while (tqueue_read(tq.queue_control, &msg) > 0);
  while (tqueue_read(tq.queue_threads, &msg) > 0) {
    if (TQMESSAGE_TEST_FILE_WORDS == msg.flag) {
      free((char**) msg.arg);
      continue;
    }
    if (TQMESSAGE_TEST_GENERATED_WORDS) {
      free((char*) msg.arg);
      continue;
    }
  }

  // Write to the console the result
  if (password != NULL) {
    printf("\n> PASSWORD : %s\n", password);
    free(password);
  } else {
    printf("\n> PASSWORD : NO MATCH\n");
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