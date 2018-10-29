#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

enum CODE { C_OPEN, C_CLOSE };


void send_code(int sid, enum CODE id) {
  static char buf;
  buf = 48 + id;
  send(sid, &buf, 1, 0);
}

int main() {
  int sock_server = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_server == -1) {
    printf("Error (%d): %s\n", errno, strerror(errno));
    return 0;
  }

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = 0; // htons(9002); // 0 getsockname();
  address.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0

  if (bind(sock_server, (struct sockaddr*) &address, sizeof(address)) != 0) {
    printf("Error (%d): %s\n", errno, strerror(errno));
    close(sock_server);
    return 0;
  }

  unsigned int address_size = sizeof(address);
  getsockname(sock_server, (struct sockaddr*) &address, &address_size);
  printf("port: %d\n", address.sin_port);

  if (listen(sock_server, 5) != 0) {
    printf("Error (%d): %s\n", errno, strerror(errno));
    close(sock_server);
    return 0;
  }

  int sock_client = accept(sock_server, NULL, NULL);
  if (sock_client == -1) {
    printf("Error (%d): %s\n", errno, strerror(errno));
    close(sock_server);
    return 0;
  }

  send_code(sock_client, C_CLOSE);

  // printf("[server] sending connected message.\n");
  // char message[256];
  // send(sock_client, message, sizeof(message), 0);

  close(sock_server);
  return 0;
}
