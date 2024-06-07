#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1000
#define MAX_DATA 10000

struct CALCDATA
{
    uint32_t data[MAX_DATA];
};

int create_data(int idx, struct CALCDATA *cdata)
{
    if (cdata != NULL)
    {
        uint32_t i;
        uint32_t *value;
        uint32_t v;
        value = &cdata->data[0];

        for (i = 0; i < MAX_DATA; i++)
        {
            v = (uint32_t)rand() ^ (uint32_t)rand();

            printf("Creating value #%d v=%u addr=%lu \r", i, v, (unsigned long)value);
            *value = v & 0x0000FFFF;
            value++;
        }
        return (1);
    }
    return (0);
}

// Function to check for errors and handle them by exiting the program
int check(int exp, const char *mess)
{
    if (exp < 0)
    {
        perror(mess);
        exit(1);
    }
    return exp;
}

struct DATA
{
    int newPort;
};

char *createPackage(char command, int port)
{
    static char result[BUFFER_SIZE];
    memset(result, ' ', BUFFER_SIZE); // Initialize the buffer with spaces

    result[0] = '@';
    strncpy(&result[1], "00000000", 8); // Placeholder for client name
    result[9] = '0';
    result[10] = '!';
    result[11] = command;
    result[12] = ':';

    switch (command)
    {
    case 'P':
        // Add the port number to the result buffer
        int written = sprintf(&result[13], "%d", port);
        result[13 + written] = '#'; // Add '#' right after the port number
        break;
    default:
        break;
    }
    return result;
}

// Function to generate response based on received command
char *responding(char *buff, int port)
{
    static char result[BUFFER_SIZE];
    char command = buff[11]; // Assuming command is always at index 11
    switch (command)
    {
    case 'N':
        // Call createPackage function to generate response
        strncpy(result, createPackage('P', port), BUFFER_SIZE);
        break;
    default:
        break;
    }
    return result;
}
int main(int argc, char *argv[])
{
    // Check if the correct number of arguments is provided in the terminal
    if (argc != 2)
    {
        printf("Usage: <port>\n");
        exit(1);
    }

    // Create and check the validity of the socket
    const int sock = socket(AF_INET, SOCK_STREAM, 0);
    check(sock, "socket() error\n");

    // Assign the port number provided in the command line
    int port = atoi(argv[1]);

    // Setting up the server structure
    struct sockaddr_in server_addr = {.sin_family = AF_INET, .sin_port = htons(port)};
    int iptn = inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
    check(iptn, "inet_pton() error\n");

    // Set socket options, allowing for reuse of the address
    int value1 = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &value1, sizeof(value1));

    // Register the service in the system
    int bnd = bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    check(bnd, "bind() error\n");

    // Listen for incoming clients
    int lst = listen(sock, MAX_CLIENTS);
    check(lst, "listen() error\n");

    printf("Server listening on port: %d\n", port);

    // Structure for storing a set of descriptors to be monitored for readiness to read
    fd_set readfds;
    int maxSock, act, newSock;
    struct sockaddr_in clientAddr;
    socklen_t clientLen;
    int clientSocks[MAX_CLIENTS] = {0};

    while (1)
    {
        // Reset bits and add the listening socket to the set
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        maxSock = sock;

        // Iterate through all clients
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int clientSock = clientSocks[i];
            if (clientSock > 0)
            {
                FD_SET(clientSock, &readfds);
            }
            if (clientSock > maxSock)
            {
                maxSock = clientSock;
            }
        }

        // Check the readiness of sockets
        act = select(maxSock + 1, &readfds, NULL, NULL, NULL);
        check(act, "select()");

        // Check if the listening socket is ready
        if (FD_ISSET(sock, &readfds))
        {
            clientLen = sizeof(clientAddr);
            newSock = accept(sock, (struct sockaddr *)&clientAddr, &clientLen);
            check(newSock, "accept()");

            printf("New connection, socket sock is %d\n", newSock);

            // Add the new socket to the client array
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clientSocks[i] == 0)
                {
                    clientSocks[i] = newSock;
                    break;
                }
            }
        }

        // Iterate through clients
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int clientSock = clientSocks[i];
            if (clientSock > 0 && FD_ISSET(clientSock, &readfds))
            {
                // otrzymany pakiet - wiadomosc od klienta
                char buff[BUFFER_SIZE];
                ssize_t bytesRec = recv(clientSock, buff, sizeof(buff), 0);

                if (bytesRec <= 0)
                {
                    if (bytesRec == 0)
                    {
                        printf("Client %d disconnected\n", clientSock);
                    }
                    else
                    {
                        perror("recv() error\n");
                    }
                    close(clientSock);
                    clientSocks[i] = 0;
                }
                else
                {

                    printf("Message from client %d:\t%s\n", clientSock, buff);
                    // wysylanie pakietu do klienta
                    // jesli
                    char outputBuff[BUFFER_SIZE];
                    strcpy(outputBuff, responding(buff, port));
                    send(clientSock, outputBuff, sizeof(outputBuff), 0);
                    printf("Message sent to client on socket %d:\t%s\n", clientSock, outputBuff);
                }
            }
        }
    }

    close(sock);
    printf("Closing server, bye!\n");
    return 0;
}
