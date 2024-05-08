#include <arpa/inet.h>  // inet_pton()
#include <netinet/in.h> // struct sockaddr_in
#include <stdio.h>
#include <stdlib.h> // exit()
#include <string.h> // memset()
#include <sys/socket.h>
#include <unistd.h> // close()

#define MAX_BUFFER_SIZE 4096
#define SERVER_IP "127.0.0.1" // IP serwera
// #define SERVER_IP "0.0.0.0" // Jeśli serwer nasłuchuje na wszystkich adresach IP
#define DEFAULT_TIMEOUT_SEC 3
#define CONNECTION_DURATION_SEC 60

int main(int argc, char *argv[])
{
    // dane wpisywane z klawiatury
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    struct sockaddr_in server_addr = {.sin_family = AF_INET, .sin_port = htons(server_port)};

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton() ERROR");
        exit(EXIT_FAILURE);
    }

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        perror("socket() ERROR");
        exit(EXIT_FAILURE);
    }

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect() ERROR");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server.\n");

    const char *message = "Hello from client";
    ssize_t bytes_sent = send(client_socket, message, strlen(message), 0);
    if (bytes_sent < 0)
    {
        perror("send() ERROR");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Message sent to server.\n");

    sleep(DEFAULT_TIMEOUT_SEC);

    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received < 0)
    {
        perror("recv() ERROR");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    else if (bytes_received == 0)
    {
        printf("Connection closed by server.\n");
        close(client_socket);
        exit(EXIT_SUCCESS);
    }

    buffer[bytes_received] = '\0';
    printf("Received message from server: %s\n", buffer);

    sleep(CONNECTION_DURATION_SEC);

    printf("Closing connection with server.\n");
    close(client_socket);

    return 0;
}
