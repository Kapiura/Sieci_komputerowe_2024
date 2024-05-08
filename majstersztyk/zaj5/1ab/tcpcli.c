//=========================================================
// Szkic dla klienta TCP
// -  przyjmuje jako argumenty <ADRES IP> <PORT> <NAZWA>
// -  laczy sie oraz wysyla <NAZWA>
// -  odbiera dane, które wypisuje sie w postaci napisow, znak \r
//      oznacza znak konca lini
//=========================================================
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
    if (argc < 3)
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
    //=========================================================
    // Deklaracja socketa TCP
    // prosze dodac wywolanie funkcji connect(<socket>,<(struct sockaddr*) adres> <dlugosc adresu>
    //=========================================================
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if ((sock) < 0)
    {
        perror("socket() ERROR");
        exit(2);
    }

    if (connect(sock, (struct sockaddr *)&serwer, sizeof(serwer)) < 0)
    {
        perror("connect() ERROR");
        exit(3);
    }
    //=========================================================
    // prosze dodac wywolanie funkcji send(<socket>,<dane>,<dlugosc danych>,<opcje>
    // by wyslac nazwe do serwera
    // opcje ustawic na 0
    //=========================================================
    if (send(sock, namebuffer, sizeof(namebuffer), 0) < 0)
    {
        printf("send() ERROR\n");
        exit(4);
    }

    //=========================================================
    // procedura odczytu w petli.
    // to jest wersja odczytu po 1 znaku
    // prosze dodac obsluge bledu gdy retval jest <0
    // prosze dodac obsluge braku odczytu retval==0
    // docelowo prosze przerobic na odczyt do bufora nie mniejszego ni¿ 200 bajtów
    //=========================================================
    //

    char buffor[256];
    int retval;
    while ((retval = recv(sock, &buffor, sizeof(buffor), 0)) > 0)
    {
        for (ssize_t i = 0; i < retval; i++)
        {
            if (retval > 0)
            {
                if (buffor[i] == '\r')
                    printf("\n");
                else
                    printf("%c", buffor[i]);
                fflush(NULL);
            }
        }
        if (retval < 0)
        {
            perror("recv() err");
            exit(1);
        }
    }
    printf("\nKoniec.\n");
    fflush(NULL);
    shutdown(sock, SHUT_RDWR);
}
