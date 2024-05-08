#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 5045
#define MAX_PENDING_CONNECTIONS 5
#define BUFFER_SIZE 1024

int main()
{
    // Tworzenie gniazda
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Ustawienie opcji gniazda SO_REUSEADDR
    int value1 = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &value1, sizeof(value1)) == -1)
    {
        perror("setsockopt");
        close(sd);
        exit(EXIT_FAILURE);
    }

    // Ustawienie adresu serwera
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(PORT);

    // Przypisanie adresu do gniazda
    if (bind(sd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        perror("bind");
        close(sd);
        exit(EXIT_FAILURE);
    }

    // Nasłuchiwanie na gnieździe
    if (listen(sd, MAX_PENDING_CONNECTIONS) == -1)
    {
        perror("listen");
        close(sd);
        exit(EXIT_FAILURE);
    }

    printf("Serwer nasłuchuje na porcie %d...\n", PORT);

    // Akceptowanie połączeń przychodzących
    struct sockaddr_in clientaddr;
    socklen_t clientaddrlen = sizeof(clientaddr);
    int sdc = accept(sd, (struct sockaddr *)&clientaddr, &clientaddrlen);
    if (sdc == -1)
    {
        perror("accept");
        close(sd);
        exit(EXIT_FAILURE);
    }

    printf("Nawiązano połączenie z klientem.\n");

    // Obsługa połączenia
    char buffer[BUFFER_SIZE];
    strcpy(buffer, "Witaj, klient!\n");

    if (send(sdc, buffer, strlen(buffer), 0) == -1)
    {
        perror("send");
        close(sdc);
        close(sd);
        exit(EXIT_FAILURE);
    }

    printf("Wiadomość została wysłana do klienta.\n");

    // Zamknięcie gniazd
    close(sdc);
    close(sd);

    return 0;
}
