#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h> // fcntl()
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define MAX_CLIENTS 5
#define BUFFER_SIZE 4096

int set_socket_nonblocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl(F_GETFL) error");
        return -1;
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("fcntl(F_SETFL) error");
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int server_port = atoi(argv[1]);

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET, .sin_port = htons(server_port), .sin_addr.s_addr = INADDR_ANY};

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("socket() error");
        exit(2);
    }

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind() error");
        exit(3);
    }

    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        perror("listen() error");
        exit(4);
    }

    if (set_socket_nonblocking(server_socket) < 0)
    {
        exit(5);
    }

    printf("Server listening on port %d\n", server_port);

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sockets[MAX_CLIENTS] = {0};
    int max_sd, activity, i, sd;
    fd_set readfds;
    char buffer[BUFFER_SIZE];

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        max_sd = server_socket;

        for (i = 0; i < MAX_CLIENTS; i++)
        {
            sd = client_sockets[i];
            if (sd > 0)
            {
                FD_SET(sd, &readfds);
            }
            if (sd > max_sd)
            {
                max_sd = sd;
            }
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
        {
            perror("select() error");
            exit(6);
        }

        if (FD_ISSET(server_socket, &readfds))
        {
            int new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
            if (new_socket < 0)
            {
                if (errno == EWOULDBLOCK)
                {
                    continue;
                }
                else
                {
                    perror("accept() error");
                    exit(7);
                }
            }

            printf("New connection established with client %s:%d\n", inet_ntoa(client_addr.sin_addr),
                   ntohs(client_addr.sin_port));

            if (set_socket_nonblocking(new_socket) < 0)
            {
                close(new_socket);
                continue;
            }

            for (i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_sockets[i] == 0)
                {
                    client_sockets[i] = new_socket;
                    break;
                }
            }
        }

        for (i = 0; i < MAX_CLIENTS; i++)
        {
            sd = client_sockets[i];
            if (FD_ISSET(sd, &readfds))
            {
                memset(buffer, 0, BUFFER_SIZE);
                int valread = recv(sd, buffer, BUFFER_SIZE, 0);
                if (valread == 0)
                {
                    printf("Client %d disconnected\n", sd);
                    close(sd);
                    client_sockets[i] = 0;
                }
                else if (valread < 0)
                {
                    perror("recv() error");
                    close(sd);
                    client_sockets[i] = 0;
                }
                else
                {
                    printf("Received data from client %d: %s\n", sd, buffer);
                    strcat(buffer, " Otrzymałem:");
                    send(sd, buffer, strlen(buffer), 0);
                }
            }
        }

        sleep(1);
    }

    close(server_socket);

    return 0;
}
#include <stdio.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERWER_IP "0.0.0.0"

int main(int argc, char *argv[])
{
    int portno;
    struct sockaddr_in server_addr;
    char namebuffer[100];
    int sock;
    int zerostate = 0;

    if (argc != 4)
    {
        printf("Usage: %s <IP> <port> <name>\n", argv[0]);
        exit(1);
    }

    portno = atoi(argv[2]);
    strncpy(namebuffer, argv[3], sizeof(namebuffer) - 1);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portno);

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton() error");
        exit(2);
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket() error");
        exit(3);
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect() error");
        exit(4);
    }

    if (send(sock, namebuffer, strlen(namebuffer) + 1, MSG_NOSIGNAL) < 0)
    {
        perror("send() error");
        exit(5);
    }

    while (1)
    {
        char c;
        int retval;

        retval = recv(sock, &c, 1, 0);

        if (retval > 0)
        {
            if (c == '\r')
            {
                printf("\n");
            }
            else
            {
                printf("%c", c);
                fflush(stdout);
            }
        }

        if (retval < 0)
        {
            printf("recv error, errno: %d\n", errno);
            break;
        }

        if (retval == 0)
        {
            zerostate++;

            if (zerostate > 20)
            {
                break;
            }
        }

        usleep(80000); // Sleep for 80 milliseconds
    }

    printf("\nConnection closed.\n");
    fflush(NULL);
    shutdown(sock, SHUT_RDWR);
    close(sock);

    return 0;
}
