#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 200 // Rozmiar bufora odbiorczego

int sock;        // socket do transmisji danych
int addr_length; // dlugosc struktury adresu

int main(int argc, char *argv[])
{
    //=========================================================
    // zmienne do przechowywania danych z lini komend
    //=========================================================
    int portno;                // numer portu
    struct sockaddr_in serwer; // adres serwera
    char namebuffer[100];      // bufor na nazwe, wysy³ana w formacie <nazwa>'\0'

    bzero((char *)&serwer, sizeof(serwer));
    bzero((char *)&namebuffer, sizeof(namebuffer));

    // pobranie z lini komend w postaci napisu
    if (argc < 4)
    {
        printf("wymagane argument: <ADRES IP> <PORT> <NAZWA>");
        exit(-1);
    }
    else
    {
        if (inet_pton(AF_INET, argv[1], &serwer.sin_addr) <= 0)
        {
            perror("Inet_pton() ERROR\n");
            exit(101);
        }
    }

    serwer.sin_family = AF_INET;
    portno = atoi(argv[2]);
    serwer.sin_port = htons(portno);

    // Przypisanie nazwy do bufora
    snprintf(namebuffer, sizeof(namebuffer), "%s", argv[3]);

    // Deklaracja socketa TCP
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket() ERROR");
        exit(2);
    }

    if (connect(sock, (struct sockaddr *)&serwer, sizeof(serwer)) < 0)
    {
        perror("connect() ERROR");
        exit(3);
    }

    // Wys¿anie nazwy do serwera
    if (send(sock, namebuffer, sizeof(namebuffer), 0) < 0)
    {
        perror("send() ERROR");
        exit(4);
    }

    // Procedura odczytu w p¿tli
    char buffer[BUFFER_SIZE]; // Bufor do odczytu danych
    ssize_t n;

    while ((n = recv(sock, buffer, sizeof(buffer), 0)) > 0)
    {
        fwrite(buffer, sizeof(char), n, stdout); // Wypisanie danych na ekran
        fflush(stdout);
    }

    if (n == 0)
    {
        printf("Socket closed by peer\n");
    }
    else if (n < 0)
    {
        perror("recv() ERROR");
        exit(EXIT_FAILURE);
    }

    printf("\nKoniec.\n");

    // Zamykanie po¿¿czenia
    shutdown(sock, SHUT_RDWR);
    close(sock);
    return 0;
}
