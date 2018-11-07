#include "args.h"
#include "talkiewalkie.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <wait.h>
#include <pthread.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <signal.h>
#include <mqueue.h>

const char TW_LOG_PREFIX[] = "<Server>";
static void* server_state_initial(int, void**);
// static void* server_state_hash(int, void*);
// static void* server_state_work(int, void*);
// static void* server_state_idle(int, void*);

static struct mq_attr mqueue_attr;
static mqd_t mqueue_jobs;
static mqd_t mqueue_resp;
struct job {
  int type;
  char* password;
};

struct work {
  int count;
  char salt[13];
  char hash[35];
  char buffer[1024];
};

struct work_order {
  struct work* work;
  char* indices[10];
};


int main(int argc, char** args) {
  // Create config with input arguments
  struct args_server_config server_config;
  if (args_server_init(&server_config, argc, args) != 0) {
    return EXIT_FAILURE;
  }
  
  // Create message queues, which will be used to communicate between threads
  mqueue_attr.mq_maxmsg = 10;
  mqueue_attr.mq_msgsize = sizeof(struct job);
  mqueue_jobs = mq_open("/cserver_jobs_queue", O_CREAT | O_RDWR, 0666, &mqueue_attr);
  mqueue_resp = mq_open("/cserver_resp_queue", O_CREAT | O_RDWR | O_NONBLOCK, 0666, &mqueue_attr);
  if (mqueue_jobs == -1 || mqueue_resp == -1) {
    fprintf(stderr, "Error opening messaging queue. errno(%d): %s\n", errno, strerror(errno));
    args_server_free(&server_config);
    exit(EXIT_FAILURE);
  }

  // Create a socket address
  struct sockaddr* server_address;
  int server_address_size;
  if (argc == 0) {
    // Creating UNIX socket address
    struct sockaddr_un* addr = calloc(1, sizeof(struct sockaddr_un));
    addr->sun_family = AF_UNIX;
    strncpy(addr->sun_path, TW_DEFAULT_UNIX_PATH, sizeof(addr->sun_path) - 1);
    server_address_size = sizeof(*addr);
    server_address = (struct sockaddr*) addr;
    unlink(TW_DEFAULT_UNIX_PATH); // hmm
  } else {
    // Creating INET socket address
    struct sockaddr_in* addr = calloc(1, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = server_config.port;
    addr->sin_addr.s_addr = (server_config.host == NULL)
      ? INADDR_ANY
      : inet_addr(server_config.host);
    server_address_size = sizeof(*addr);
    server_address = (struct sockaddr*) addr;
  }
  args_server_free(&server_config);

  // Creating a socket
  int server_socket = socket(server_address->sa_family, SOCK_STREAM, 0);
  if (server_socket == -1) {
    fprintf(stderr, "Error opening socket. errno(%d): %s\n", errno, strerror(errno));
    free(server_address);
    exit(EXIT_FAILURE);
  }

  // Binding socket to address
  if (bind(server_socket, server_address, server_address_size) != 0) {
    fprintf(stderr, "Error binding socket. errno(%d): %s\n", errno, strerror(errno));
    close(server_socket);
    free(server_address);
    exit(EXIT_FAILURE);
  }
  printf("<Server> socket bound %d\n", server_socket);

  // Listen for connections
  if (listen(server_socket, 5) != 0) {
    fprintf(stderr, "Error listening on socket. errno(%d): %s\n", errno, strerror(errno));
    close(server_socket);
    free(server_address);
    exit(EXIT_FAILURE);
  }
  printf("<Server> socket listening %d\n", server_socket);

  // Attemt to accept incoming client connections
  // Only accepting one client at a time, dont see the need
  // to support multiple users at the same time
  // pthread_t thread;
  // pthread_create(&thread, NULL, server_state_accept, &server_socket);
  // pthread_join(thread, NULL);

  int client_socket = accept(server_socket, NULL, NULL);
  if (client_socket == -1) {
    fprintf(stderr, "Error accepting client connection. errno(%d): %s\n", errno, strerror(errno));
    close(server_socket);
    free(server_address);
    exit(EXIT_FAILURE);
  }
  printf("<Server> client (%d) connected\n", client_socket);

  // start talkiewalkie state loop
  tw_state_fn fn = &server_state_initial;
  tw_send_code(client_socket, TW_CODE_IDLE);
  while (fn != NULL) {
    fn = (tw_state_fn)(*fn)(client_socket, NULL);
  }
  printf("<Server> client (%d) disconnected\n", client_socket);

  close(client_socket);
  close(server_socket);
  free(server_address);
  printf("<Server> exiting.\n");
  return EXIT_SUCCESS;
}

static void* server_state_initial(int client, void** state) {
  int code;
  if (tw_read_code(client, &code) == 0) return NULL;
  if (code != TW_CODE_HASH) return NULL;

  // infinitely read all and just drop it
  char buf[128];
  while (1) {
    int bytes = recv(client, &buf, 128, MSG_WAITALL);
    if (bytes == 0) return NULL;
    printf("read: %s\n", &buf);
  }
  

  /*
  // read the hash
  char hash[35];
  if (recv(client, hash, 34, MSG_WAITALL) == 0) return NULL;

  // write the buffer size
  char buf[9];
  snprintf(buf, 8, "[%05d]", 1024);
  send(client, &buf, 8, 0);

  return NULL;
  // *state = malloc(sizeof(struct work));
  // *state->index = malloc(sizeof())
  */
}

// static void* server_state_hash(int client, void* state) {}
// static void* server_state_work(int client, void* state) {}
// static void* server_state_idle(int client, void* state) {}

/*
static void* server_state_idle(int client, void* state) {
  int code;
  if (tw_read_code(client, &code) == 0) return NULL;
  switch (code) {
    case TW_CODE_HASH: {
      memcpy(salt, hash, 12);
      printf("<Server> SALT: %s\n", salt);
      printf("<Server> HASH: %s\n", hash);
      break;
    }
    case TW_CODE_WORDS:
      if (state == NULL) {
        printf("");
        return NULL;
      }
    case TW_CODE_EXIT: {
      return NULL;
    default:
      printf("<Server> IDLE: unhandled code %d\n", code);
      return NULL;
  }
  return &server_state_idle;
}
*/

/*
static void* server_state_accept(void* arg) {
  int server_socket = *(int*) arg;
  int client_socket;
  struct sockaddr_in client_address;
  socklen_t client_address_size = sizeof(struct sockaddr_in);
  while (1) {
    printf("??\n");
    client_socket = accept(server_socket, (struct sockaddr*) &client_address, &client_address_size);
    printf("!!\n");
    if (client_socket == -1) {
      fprintf(stderr, "Error accepting client connection. errno(%d): %s\n", errno, strerror(errno));
      close(server_socket);
      pthread_exit(0);
      // return EXIT_FAILURE;
    }
    printf("<Server> client (%s) connected\n", inet_ntoa(client_address.sin_addr));

    // start talkiewalkie state loop
    tw_state_fn fn = &server_state_idle;
    tw_send_code(client_socket, TW_CODE_IDLE);
    while (fn != NULL) {
      fn = (tw_state_fn)(*fn)(client_socket);
    }

    // close client connection
    printf("<Server> client (%s) disconnected\n", inet_ntoa(client_address.sin_addr));
    close(client_socket);
    break;
  }
  pthread_exit(0);
}
*/