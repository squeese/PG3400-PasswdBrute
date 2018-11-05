#include "args.h"
#include "talkiewalkie.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <wait.h>
#include <pthread.h>
#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/prctl.h>

const char TW_LOG_PREFIX[] = "<Client>";
static void* server_thread_handler(void*);
static void* server_state_send_hash(int, void**);

static void* job_thread_handler(void*);

static char hash[35];

static struct args_client_config client_config;
static struct mq_attr mqueue_jobs_attr;
static mqd_t mqueue_jobs;
struct server_job {
  int type;
  char* password;
};

static struct mq_attr mqueue_resp_attr;
static mqd_t mqueue_resp;
struct server_resp {
  unsigned int id;
  unsigned int code;
  char* data;
};
enum {
  SERVER_RESPONSE_CONNECTED,
  SERVER_RESPONSE_DISCONNECTED,
  SERVER_RESPONSE_FUCK,
};

int main(int argc, char** args) {
  // Create config with input arguments
  if (args_client_init(&client_config, argc, args) != 0) {
    return EXIT_FAILURE;
  }

  // Set the salt/hash provided in the global scope
  memcpy(hash, client_config.hash, 34);

  // Check if we need to start a local server if there isnt one
  // specified in the config provided by the user
  if (client_config.servers == NULL) {
    pid_t process_id = fork();
    if (process_id == 0) {
      // Spawning a local server
      prctl(PR_SET_PDEATHSIG, SIGHUP);
      static char* args[] = {};
      execv("./server", args);
      fprintf(stderr, "Error forking child server process. errno(%d): %s\n", errno, strerror(errno));
      exit(EXIT_FAILURE);
    } else {
      // Creating a socket address to connect to the local server
      struct sockaddr_un* addr = calloc(1, sizeof(struct sockaddr_un));
      addr->sun_family = AF_UNIX;
      strncpy(addr->sun_path, TW_DEFAULT_UNIX_PATH, sizeof(addr->sun_path) - 1);
      args_client_push_server(&client_config, (struct sockaddr*) addr);
    }
  }

  // Create message queues, which will be used to communicate between threads
  mqueue_jobs_attr.mq_maxmsg = client_config.num_servers * 2;
  mqueue_jobs_attr.mq_msgsize = sizeof(struct server_job);
  mqueue_resp_attr.mq_maxmsg = client_config.num_servers;
  mqueue_resp_attr.mq_msgsize = sizeof(struct server_resp);
  mqueue_jobs = mq_open("/cclient_jobs_queue", O_CREAT | O_RDWR, 0666, &mqueue_jobs_attr);
  mqueue_resp = mq_open("/cclient_resp_queue", O_CREAT | O_RDWR | O_NONBLOCK, 0666, &mqueue_resp_attr);
  if (mqueue_jobs == -1 || mqueue_resp == -1) {
    fprintf(stderr, "Error opening messaging queue. errno(%d): %s\n", errno, strerror(errno));
    args_client_free(&client_config);
    exit(EXIT_FAILURE);
  }

  // Spawn threads who will manage the communication to the servers
  // Will read from the messaging queues jobs, send to servers, read response and put
  // them back into the response queue
  pthread_t* server_threads = malloc(client_config.num_servers * sizeof(pthread_t));
  for (int i = 0; i < client_config.num_servers; i++) {
    pthread_create(server_threads + i, NULL, server_thread_handler, *(client_config.servers + i));
  }

  // Wait for servers to connect
  struct server_resp resp;
  for (int i = 0; i < client_config.num_servers; ) {
    int status = mq_receive(mqueue_resp, (char*) &resp, sizeof(struct server_resp), NULL);
    if (status > 0) {
      if (resp.code == SERVER_RESPONSE_DISCONNECTED) {
        printf("server failed to connect\n");
        for (int j = 0; j < client_config.num_servers; j++) {
          pthread_cancel(*(server_threads + j));
        }
        args_client_free(&client_config);
        exit(EXIT_FAILURE);
      }
      printf("Server connected, good\n");
      i++;
    }
    usleep(100000);
  }
  printf("<Client> all servers connected\n");

  struct words_dictionary dictionary;
  words_dictionary_init(&words_dictionary);

  struct words_permutation permutation;
  words_permutation_init(&words_permutation, client_config.length);

  words_next(&words, 32);

  // etc
  int response_size;
  while (1) {
    // check for responses, this is a non-blocking queue
    int response_size = mq_receive(mqueue_resp, (char*) &resp, sizeof(struct server_resp), NULL);
    if (response_size > 0) {
      // handle disconnect etc
    }

    // create jobs to send to servers

  }

  // Spawn a thread to create jobs for the servers, will partition the dictionary and
  // words generated, and let servers take jobs as they finish stuff
  // pthread_t job_thread;
  // pthread_create(&job_thread, NULL, job_thread_handler, NULL);

  // pthread_join(job_thread, NULL);

  // We now wait for the server threads to exit
  // for (int i = 0; i < client_config.num_servers; i++)
    // pthread_join(*(server_threads + i), NULL);
  // free(server_threads);

  // and stop the working thread spawning jobs
  // pthread_cancel(job_thread);
  args_client_free(&client_config);
  // mq_unlink

  // Phew.
  printf("<Client> exit \n");
  return EXIT_SUCCESS;
}

static void server_thread_cleanup(void* arg) {
  printf("closing server thread\n");
  close(*(int*) arg);
}

static void* server_thread_handler(void* arg) {
  // Some casting depending on the kind of address
  // Its either via unix socket file, or interwebs yes
  struct sockaddr* address = (struct sockaddr*) arg;
  int address_size = (address->sa_family == AF_UNIX)
    ? sizeof(*(struct sockaddr_un*) address)
    : sizeof(*(struct sockaddr_in*) address);

  // Create a socket
  struct server_resp resp = { NULL, server, NULL };
  int server = socket(address->sa_family, SOCK_STREAM, 0);
  if (server == -1) {
    fprintf(stderr, "Error creating socket. errno(%d): %s\n", errno, strerror(errno));
    resp.code = SERVER_RESPONSE_DISCONNECTED;
    mq_send(mqueue_resp, (const char*) &resp, sizeof(struct server_resp), 1);
    pthread_exit(0);
  }
  pthread_cleanup_push(&server_thread_cleanup, &server);
  resp.code.SERVER_RESPONSE_CONNECTED;
  mq_send(mqueue_resp, (const char*)&resp, sizeof(struct server_resp), 1);

  // Connect to server via socket
  for (int attempts = 3; attempts >= 0; attempts--) {
    if (connect(server, address, address_size) != -1) break;
    if (attempts <= 0) {
      fprintf(stderr, "Error connecting to server %d. errno(%d): %s\n", server, errno, strerror(errno));
      // close(server);
      pthread_exit(0);
    }
    printf("<Client> retrying server %d\n", server);
    sleep(1);
  }
  printf("<Client> connected to server %d\n", server);

  // Start the TALKIEWALKIE state loop
  tw_state_fn fn = &server_state_send_hash;
  while (fn != NULL) {
    fn = (tw_state_fn)(*fn)(server, NULL);
  }

  printf("<Client> socket closed\n");
  pthread_cleanup_pop(0);
  close(server);
  pthread_exit(0);
}

static void* server_state_send_hash(int server, void** state) {
  int code;
  if (tw_read_code(server, &code) == 0) return NULL;
  if (code != TW_CODE_IDLE) return NULL;

  // issue hash 
  tw_send_code(server, TW_CODE_HASH);



/*
  // send hash
  // write(server, hash, 34);
  // read expected buffer size
  char buf[9];
  recv(server, &buf, 8, MSG_WAITALL);
  int size = atoi(&buf[1]);

  printf("size: %s, %d\n", &buf[0], size);
  */

  return NULL;
}

  /*
static void* job_thread_handler(void* arg) {
  // 1) solve hash via dictionary
  char * line = NULL;
  size_t len = 0;
  FILE * fp = fopen(client_config.dictionary, "r");
  if (fp == NULL) {
    printf("Unable to open dictionary file at given path: %s\nError: (%d) %s\n", path, errno, strerror(errno));
    return errno;
  }

  // 2) solve hash via generating words




  int status;
  while (1) {
    struct job msg;
    status = mq_receive(mqueue_resp, (char*) &msg, sizeof(struct job), NULL);
    if (status > 0) {
      printf("Received in job %d\n", msg.type);
    } else {
      printf("Failed in job\n");
      sleep(100);
      pthread_exit(0);
    }
  }
  sleep(10);
}
  */