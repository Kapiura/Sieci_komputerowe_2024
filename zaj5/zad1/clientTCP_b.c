#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int sock;        // socket do transmisji danych
int addr_length; // długość struktury adresu

int main(int argc, char *argv[]) {

  //=========================================================
  // zmienne do przechowywania danych z lini komend
  //=========================================================
  int portno;                // numer portu
  struct sockaddr_in serwer; // adres serwera

  bzero((char *)&serwer, sizeof(serwer));

  if (argc < 3) {
    fprintf(stderr, "Usage: %s <Ip addr> <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  if (inet_pton(AF_INET, argv[1], &serwer.sin_addr) <= 0) {
    perror("inet_pton() ERROR");
    exit(EXIT_FAILURE);
  }

  serwer.sin_family = AF_INET;
  portno = atoi(argv[2]);
  serwer.sin_port = htons(portno);

  //=========================================================
  // Deklaracja socketa TCP
  //=========================================================
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("socket() ERROR");
    exit(EXIT_FAILURE);
  }

  addr_length = sizeof(serwer);
  if (connect(sock, (struct sockaddr *)&serwer, addr_length) < 0) {
    perror("Connect() error\n");
    exit(EXIT_FAILURE);
  }

  //=========================================================
  // procedura odczytu w petli.
  //=========================================================
  char buffer[200];
  ssize_t n;

  while ((n = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
    fwrite(buffer, sizeof(char), n, stdout);
    fflush(stdout);
  }
  if (n < 0) {
    perror("recv() ERROR");
    exit(EXIT_FAILURE);
  }

  printf("\nKoniec.\n");
  fflush(stdout);
  shutdown(sock, SHUT_RDWR);
  close(sock);
  return 0;
}
