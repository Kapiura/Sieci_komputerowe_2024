

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define SERWER_PORT 2137
#define SERWER_IP "156.17.148.219"

int main()
{
    // W strukturze server_addr bÄ™dzie adres serwera UDP
    struct sockaddr_in serwer_addr = {.sin_family = AF_INET, .sin_port = htons(SERWER_PORT)};
    if (inet_pton(AF_INET, SERWER_IP, &serwer_addr.sin_addr) <= 0)
    {
        printf("inet_pton() ERROR\n");
        exit(101); // exit - 101 to kod bÅ‚edu
    }

    int sd = socket(AF_INET, SOCK_DGRAM, 0);

    // char buffer[4096] = "Welcome udp server ;33";
    char buffer[8000] = "Welcome udp server ";

    printf("Message for server: %s \n", buffer);

    socklen_t len = sizeof(serwer_addr);

    if (sendto(sd, buffer, strlen(buffer), 0, (struct sockaddr *)&serwer_addr, len) < 0)
    {
        printf("sendto() ERROR\n");
        exit(103);
    }

    struct sockaddr_in from = {};
    memset(buffer, 0, sizeof(buffer));

    int received = recvfrom(sd, buffer, 7000, 0, (struct sockaddr *)&from, &len);
    if (received < 0)
    {
        printf("recvfrom() ERROR\n");
        exit(104);
    }

    /****************************************************************
    **   prosze wypisac:
    **
    **    - dÅ‚ugoÅ›Ä‡ otrzymanych danych
    **    - adres  nadawcy jako string np inet_ntoa(from.sin_addr)
    **    - zawartosc pakietu odebranego jako string
    ****************************************************************/
    printf("Dlugosc otrzymanych danych:\t%d\n", (int)sizeof(buffer));
    printf("Adres nadawcy:\t%s\n", inet_ntoa(from.sin_addr));
    printf("Zawartość pakietu odebranego jako string:\t%s\n", buffer);

    shutdown(sd, SHUT_RDWR);
}
