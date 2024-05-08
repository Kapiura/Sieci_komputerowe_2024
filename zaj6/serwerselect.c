#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#define PORT 5045
#define MAX_PENDING_CONNECTIONS 20
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

    // Inicjalizacja zbioru deskryptorów
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sd, &readfds);
    int max_sd = sd;

    // Akceptowanie połączeń przychodzących
    struct sockaddr_in clientaddr;
    socklen_t clientaddrlen = sizeof(clientaddr);
    int client_sockets[MAX_PENDING_CONNECTIONS]; // Tablica dla deskryptorów klientów
    int activity, i, valread, sd_new, max_clients = MAX_PENDING_CONNECTIONS;

    // Inicjalizacja tablicy deskryptorów klientów
    for (i = 0; i < max_clients; i++)
    {
        client_sockets[i] = 0;
    }

    while (1)
    {
        fd_set tmpfds = readfds;

        // Wywołanie select
        activity = select(max_sd + 1, &tmpfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            perror("select");
            exit(EXIT_FAILURE);
        }

        // Nowe połączenie przychodzące
        if (FD_ISSET(sd, &tmpfds))
        {
            if ((sd_new = accept(sd, (struct sockaddr *)&clientaddr, &clientaddrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            printf("Nowe połączenie, deskryptor gniazda: %d, IP: %s, Port: %d\n", sd_new,
                   inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

            // Dodanie nowego deskryptora klienta do zbioru deskryptorów
            for (i = 0; i < max_clients; i++)
            {
                if (client_sockets[i] == 0)
                {
                    client_sockets[i] = sd_new;
                    break;
                }
            }

            // Dodanie nowego deskryptora klienta do zbioru deskryptorów
            FD_SET(sd_new, &readfds);

            // Uaktualnienie maksymalnego deskryptora
            if (sd_new > max_sd)
                max_sd = sd_new;

            // Brak więcej deskryptorów do sprawdzenia
            if (--activity <= 0)
                continue;
        }

        // Obsługa danych od klientów
        for (i = 0; i < max_clients; i++)
        {
            sd_new = client_sockets[i];

            if (FD_ISSET(sd_new, &tmpfds))
            {
                if ((valread = read(sd_new, buffer, BUFFER_SIZE)) == 0)
                {
                    // Zamknięcie połączenia
                    getpeername(sd_new, (struct sockaddr *)&clientaddr, &clientaddrlen);
                    printf("Host o IP %s i porcie %d się rozłączył\n", inet_ntoa(clientaddr.sin_addr),
                           ntohs(clientaddr.sin_port));
                    close(sd_new);
                    client_sockets[i] = 0;
                    FD_CLR(sd_new, &readfds);
                }
                else
                {
                    // Obsługa danych od klienta
                    buffer[valread] = '\0';
                    printf("Otrzymano od klienta %d: %s\n", sd_new, buffer);

                    // Odpowiedź do klienta
                    if (send(sd_new, buffer, strlen(buffer), 0) == -1)
                    {
                        perror("send");
                    }
                }

                // Brak więcej deskryptorów do sprawdzenia
                if (--activity <= 0)
                    break;
            }
        }
    }

    // Zamknięcie gniazd
    close(sd);

    return 0;
}
