#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()

/*
 * struct sockaddr_in
 * {
 * short sin_family;            jest to rodzina adresów IP my zawsze podajemy AF_INET dla IPv4
 * unsigned short sin_port;     port w reprezentacji sieciowej
 * struct in_addr sin_addr;     adres IP w formie liczby
 * char sin_zero[8];            cos co nie ma aktualnie znaczenia
 * };
 */

int main(int argc, char *argv[])
{
    // pobranie parametru z linii komend
    if (argc != 4)
    {
        printf("Usage: <name> <server address> <port numer>");
        exit(1);
    }

    // numer portu
    int port = atoi(argv[3]);
    // dane serwera z ktorym pragniemy sie polaczyc
    struct sockaddr_in serverAddr;
    // adres ip serwera
    char *ip_addr = argv[2];
    // Stworzenie struktury serwera
    serverAddr.sin_family = AF_INET;

    bzero((char *)&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip_addr);

    // Tworzenie gniazda TCP
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket() error\n");
        exit(3);
    }

    // tworzenie polaczenia klienta z serwerem za pomoca connect
    // polaczenie sokcetu - connect
    int conn = connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (conn < 0)
    {
        printf("connect() ERROR");
        exit(4);
    }

    // nazawa klienta podana w wierszu polecen
    char namebuffer[100];
    strcpy(namebuffer, argv[1]);

    // komunikacja z serwerem
    char buffor[128];
    int n;

    while (1)
    {
        bzero(buffor, sizeof(buffor));

        printf("Type mess for server:\n");
        n = 0;

        while ((buffor[n++] = getchar()) != '\n')
            ;
        write(sock, buffor, sizeof(buffor));

        bzero(buffor, sizeof(buffor));

        read(sock, buffor, sizeof(buffor));

        printf("Mess from server:\n");
        printf("%s\n", buffor);

        if ((strcmp(buffor, "q")) == 0)
        {
            printf("Bye!");
            break;
        }
    }

    // zamkniecie gniazda
    close(sock);
    return 0;
}

/*
 * KLIENT TCP
 * ODBIERA DANE OD SERWERA
 * WYKONUJE OBLICZENIA
 * WYSYLA WYNIKI DO SERWERA
 *
 * KLIENT URUCHAMIANY Z PARAMETRAMI
 * <nazwa> <adres serwera> <nr portu>
 *
 * ZA POMOCA SKYRPTU W DOCELOWYM TESCIE
 * URUCHAMIAMY DO 10 KLIENTOW
 *
 * KLIENT LACZY SIE Z SERWEREM ZA POMOCA
 * ADRESU I PORTU
 *
 * COMMAND
 *
 * PO POLACZENIU KLIENT WYSYLA DO SERWERA
 * PAKIET A1 Z NAZWA PODANA JAKO PARAMETR
 * I KOMENDA N => PROSBA O NOWE DANE
 *
 * KLIENT PO DOSTANIU KOMENDY P OTWIERA POLACZENIE DATA
 * NA WSKAZANY NUMER PORITU I POBIERA DANE
 * WYKONUJE OBLICZENIA
 * ODSYLA WYIKI W PAKIECIE A2 Z KOMENDA R
 * JESLI WYSTAPI BLAD, KLIENT WYSYLA DO SERWERA PAKIET A3
 *
 * Z KOMENDA E
 *
 * PO OTRZYMANIU KOMENDY X KONCZY PRACE KLIENT
 *
 * KLIENT PO OTRZYMANIU D KONCZY POALCZAENIE COMMAND I ROZOPOCZYNA PRACE OD NOWA
 *
 * NA EKRANIE MUSZA SIE POJAWIC INFORMACJE O STANIE POLACZNEIA I WYNIKACH DANYCH
 */
