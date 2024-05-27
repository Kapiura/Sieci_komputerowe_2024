#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()
#include <unistd.h>

#define MAX_CONNECTION 10
#define MESSAGE_LEN 128
#define SERVER_IP "127.0.0.1"

// checking
int check(int exp, const char *mess)
{
    if (exp < 0)
    {
        perror(mess);
        exit(1);
    }
    return exp;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: <port>");
        exit(1);
    }

    // Utworzenie oraz sprawdzenie poprawnosci gniazda
    const int sock = socket(AF_INET, SOCK_STREAM, 0);
    check(sock, "socket() error\n");

    // Przypisanie numeru portu podanemu w linii komend
    int port = atoi(argv[1]);

    // ustawienie struktury serwera
    struct sockaddr_in server_addr = {.sin_family = AF_INET, .sin_port = htons(port)};
    int iptn = inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
    check(iptn, "inet_pton() error\n");

    // rejestrowanie uslugi w systemie
    int bnd = bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    check(bnd, "bind() error\n");

    // nasluchiwanie laczacyc sie klientow
    int lst = listen(sock, MAX_CONNECTION);
    check(lst, "listen() error\n");

    socklen_t len = sizeof(server_addr);
    const int stdinWalue = 0;

    while (1)
    {
        fd_set readfds;

        FD_ZERO(&readfds);
        FD_SET(stdinWalue, &readfds);

        struct timeval tv = {2};

        switch (select(stdinWalue + 1, &readfds, NULL, NULL, &tv))
        {
        case -1:
            perror("select");
            return 1;
        case 0:
            puts("Nic nie wpisano na standardowe wejscie");
            break;
        default:
            if (FD_ISSET(stdinWalue, &readfds))
            {
                char buffer[100] = {};
                scanf("%100s", buffer);
                printf("Podano tekst \"%s\"!\n", buffer);
            }
            else
                printf("Zmieniono inny deskryptor\n");

            break;
        }
    }

    // while (1)
    // {
    //     // struktura klienta
    //     struct sockaddr_in client = {};
    //
    //     // akceptowanie klietna
    //     const int clientSocket = accept(sock, (struct sockaddr *)&client, &len);
    //     check(clientSocket, "accept() error\n");
    //
    //     // wiadomosc przekazywana
    //     char buffor[MESSAGE_LEN] = {};
    //
    //     // odczytywanie bufforu od klienta
    //     int rcv = recv(clientSocket, buffor, sizeof(buffor), 0);
    //     check(rcv, "recv() error\n");
    //
    //     // drukkowanie wiadomosci od klienta
    //     printf("Mess from client:\n");
    //     printf("%s\n", buffor);
    //
    //     strcpy(buffor, "Message from server:\n");
    //     // wysylanie wiadomosci od serwera
    //     int snd = send(clientSocket, buffor, strlen(buffor), 0);
    //     check(snd, "send() error()\n");
    //
    //     int opt = 1;
    //     setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    // }
    // shutdown(sock, SHUT_RDWR);
    return 0;
}
/*
 // * SERWER TCP
 * TWORZY I UDOSTEPNIA DANE DO OBLICZEN
 * ODBIERA WYNIKI OD KLIENTA
 *
 * MUSI OBSLUGIWAC 10 POLACZEN JEDNOCZESNIE
 * ZA POMOCA SELECTA
 *
 // * POWINNEN BYC ZREALIZOWANY NA GNIAZDACH BLOKUJACYCH
 *
 * DANE DO OBLICZEN - TWORZONE PRZEZ SERWER
 * FORMAT unit32_
 * zakres 0 - 65535
 * FUNKCJA GENERUJACA DANE
 * PODANA W OSOBNYM DOKUMENCIE
 * NALEZY WLACZYC DO KODU - WYWOLAC PRZY KAZDYM TWORZENIU
 * NOWEGO ZESTAWU DANYCH
 *
 * OBLICZYC ILOSC WYSTAPIEN BITOW
 * NA POSZCZEGOLNYCH POZYCJACH W WARTOSCIACH WYGENEROWANYCH PRZEZ
 * SERWER bity 0-15
 *
 * SERWER URUCHAMIANY Z PARAMETRAMI
 * <nr portu nasluchujacego>
 *
 *
 * OTWIERA GNIAZDO NASLUCHUJACE TCP NA NUMERZE PORTU Z PARAMETRU
 *
 * SERWER ODPOWIADA NA PAKIET A1 PAKIETEM B1 Z KOMENDA P ORAZ NUMEREM PORTU
 * LUB PAKIETEM B2 Z KOMENDA X -  BRAK NOWYCH DANYCH
 *
 * PRZED WYSLANIEM B1 SERWER OTWIERA GNIAZDKO NASLUCHUJACE NA PORCIE
 * BEDZIE WYLICZANE NA PODSTAWIE PORT_NASLUCHUJACY + 1 + NUMER_ZESTAWU_DANYCH
 *
 * SERWER PO KOMENDZIE E UZYJE BLOKU DANYCH W KOLEJNYM POLACZENIU
 *
 * SERWER PO OTRZYMANIU WYNIKU SPRAWDZA WARTOSCI Z OTRZYMANMI OD KLIENTA
 * WYPISUJE INFO CZY WYNIKI SIE ZGADZAJA
 *
 * PO WYNIKACH SERWER WYSYLA DO KLIENTA PAKIET B3 Z KOEMNDA D I KONCZY POLACZENIE
 */
