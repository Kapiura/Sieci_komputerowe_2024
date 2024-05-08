#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define SERWER_PORT 8600
#define SERWER_IP "127.0.0.1"

int main()
{
    // W strukturze server_addr będzie adres serwera UDP
    struct sockaddr_in serwer_addr = {.sin_family = AF_INET, .sin_port = htons(SERWER_PORT)};
    if (inet_pton(AF_INET, SERWER_IP, &serwer_addr.sin_addr) <= 0)
    {
        printf("inet_pton() ERROR\n");
        exit(101); // exit - 101 to kod błedu
    }

    // otwarcie socketa dla klienta UDP
    int sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sd < 0)
    {
        printf("socket() ERROR\n");
        exit(102);
    }
    // DLUGOSC ADRESU
    socklen_t len = sizeof(serwer_addr);
    // wyswieltanie tekstu dla serwera
    char buffer[4096] = "<Pozdro serwer>";
    printf("Message for server: %s\n", buffer);
    // wysyłanie pakietu
    if (sendto(sd, buffer, strlen(buffer), 0, (struct sockaddr *)&serwer_addr, len) < 0)
    {
        printf("sendto() ERROR\n");
        exit(103);
    }
    // ODBIERANIE PAKIETU
    struct sockaddr_in from = {};
    memset(buffer, 0, sizeof(buffer));

    int received = recvfrom(sd, buffer, sizeof(buffer), 0, (struct sockaddr *)&from, &len);
    if (received < 0)
    {
        printf("recvfrom() ERROR\n");
        exit(104);
    }

    printf("DLUGOSC OTRZYMYWANYCH DANYCH: %d", (int)sizeof(buffer));
    printf("ADRES NADAWCY JAKO STRING: %s", inet_ntoa(from.sin_addr));
    printf("ZAWARTOSC PAKIETU ODEBRANEGO JAKO STRING: %s", buffer);

    /****************************************************************
    **   prosze wypisac:
    **
    **    - długość otrzymanych danych
    **    - adres  nadawcy jako string np inet_ntoa(from.sin_addr)
    **    - zawartosc pakietu odebranego jako string
    ****************************************************************/

    shutdown(sd, SHUT_RDWR);
}
