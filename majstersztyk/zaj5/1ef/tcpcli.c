#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 200

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        fprintf(stderr, "Usage: %s <IP> <PORT> <NAME>\n", argv[0]);
        return 1;
    }

    int sockfd;
    struct sockaddr_in server_addr;
    char namebuffer[100];
    bzero((char *)&server_addr, sizeof(server_addr));
    bzero((char *)&namebuffer, sizeof(namebuffer));

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton() ERROR");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    snprintf(namebuffer, sizeof(namebuffer), "%s", argv[3]);

    sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sockfd < 0)
    {
        perror("socket() ERROR");
        return 1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        if (errno != EINPROGRESS)
        {
            perror("connect() ERROR");
            return 1;
        }
    }

    fd_set readfds;
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

    int select_result = select(sockfd + 1, &readfds, NULL, NULL, &timeout);
    if (select_result < 0)
    {
        perror("select() ERROR");
        return 1;
    }
    else if (select_result == 0)
    {
        printf("Timeout while connecting\n");
        return 1;
    }

    if (send(sockfd, namebuffer, sizeof(namebuffer), 0) < 0)
    {
        perror("send() ERROR");
        return 1;
    }

    char buffer[BUFFER_SIZE];
    ssize_t n;

    while ((n = recv(sockfd, buffer, sizeof(buffer), 0)) > 0)
    {
        fwrite(buffer, sizeof(char), n, stdout);
        fflush(stdout);
    }

    if (n == 0)
    {
        printf("Socket closed by peer\n");
    }
    else if (n < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            printf("No more data available for reading\n");
        }
        else
        {
            perror("recv() ERROR");
        }
        return 1;
    }

    printf("\nKoniec.\n");
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    return 0;
}
