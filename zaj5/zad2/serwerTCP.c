#include <arpa/inet.h>  // inet_pton()
#include <netinet/in.h> // struct sockaddr_in
#include <stdio.h>
#include <stdlib.h> // exit()
#include <string.h> // memset()
#include <sys/socket.h>
#include <sys/time.h> // struct timeval
#include <unistd.h>   // close()

#define MAX_CLIENTS 5

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    exit(1);
  }

  int server_port = atoi(argv[1]);

  struct sockaddr_in server_addr = {.sin_family = AF_INET,
                                    .sin_port = htons(server_port),
                                    .sin_addr.s_addr = INADDR_ANY};

  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket < 0) {
    perror("socket() error");
    exit(2);
  }

  if (bind(server_socket, (struct sockaddr *)&server_addr,
           sizeof(server_addr)) < 0) {
    perror("bind() error");
    exit(3);
  }

  if (listen(server_socket, MAX_CLIENTS) < 0) {
    perror("listen() error");
    exit(4);
  }

  printf("Server listening on port %d\n", server_port);

  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  int client_socket;

  while (1) {
    client_socket =
        accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
    if (client_socket < 0) {
      perror("accept() error");
      exit(5);
    }

    printf("Connection established with client %s:%d\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    sleep(3); // Wait for 3 seconds

    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    if (recv(client_socket, buffer, sizeof(buffer), 0) < 0) {
      perror("recv() error");
      close(client_socket);
      continue;
    }
    printf("Received data from client: %s\n", buffer);

    // Sending some data back
    char response[] = "Data from server: Thanks for connecting!";
    if (send(client_socket, response, strlen(response), 0) < 0) {
      perror("send() error");
      close(client_socket);
      continue;
    }

    // Wait for 1 minute
    printf("Waiting for 1 minute before closing the connection...\n");
    sleep(60);

    printf("Closing connection with client %s:%d\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    close(client_socket);
  }

  close(server_socket);

  return 0;
}
