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
  char namebuffer[100];      // bufor na nazwe, wysyłana w formacie <nazwa>'\0'

  bzero((char *)&serwer, sizeof(serwer));
  bzero((char *)&namebuffer, sizeof(namebuffer));

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <Ip addr> <port> <name>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  if (inet_pton(AF_INET, argv[1], &serwer.sin_addr) <= 0) {
    perror("inet_pton() ERROR");
    exit(EXIT_FAILURE);
  }

  serwer.sin_family = AF_INET;
  portno = atoi(argv[2]);
  serwer.sin_port = htons(portno);

  // Copy name from command-line argument to namebuffer
  // snprintf(namebuffer, sizeof(namebuffer), "%s", argv[3]);

  //=========================================================
  // Deklaracja socketa TCP
  // prosze dodac wywolanie funkcji connect(<socket>,<(struct sockaddr*) adres>
  // <dlugosc adresu>
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
  // prosze dodac wywolanie funkcji send(<socket>,<dane>,<dlugosc
  // danych>,<opcje> by wyslac nazwe do serwera opcje ustawic na 0
  //=========================================================
  if (send(sock, namebuffer, sizeof(namebuffer), 0) < 0) {
    perror("send() error\n");
    exit(EXIT_FAILURE);
  }

  //=========================================================
  // procedura odczytu w petli.
  // docelowo prosze przerobic na odczyt do bufora nie mniejszego niż 200 bajtów
  //=========================================================
  char buffer[200];
  ssize_t n;

  while ((n = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
    for (ssize_t i = 0; i < n; i++) {
      if (buffer[i] == '\r')
        printf("\n");
      else
        printf("%c", buffer[i]);
    }
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
