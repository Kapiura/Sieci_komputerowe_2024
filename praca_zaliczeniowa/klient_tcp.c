#include <arpa/inet.h> // inet_addr()
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()

#define BUFFER_SIZE 1000

uint32_t histogram[16];

void clear_histogram()
{
    bzero(histogram, sizeof(histogram));
}

char *sendPackage(char *nameBuff, char command)
{
    static char result[BUFFER_SIZE];
    // memset(result, '0', BUFFER_SIZE); // Initialize the buffer with null characters
    //
    size_t nameLen = strlen(nameBuff);
    result[0] = '@';
    if (nameLen < 8)
    {
        char paddedName[9] = {0};
        strncpy(paddedName, nameBuff, nameLen);
        memset(paddedName + nameLen, '_', 8 - nameLen);
        strncpy(&result[1], paddedName, 8);
    }
    else
    {
        strncpy(&result[1], nameBuff, 8);
    }
    result[9] = '0'; // Ensure the '0' remains at index 9
    result[10] = '!';
    result[11] = command;
    result[12] = ':';

    // Handle any specific commands if necessary
    switch (command)
    {
    case 'N':
        result[13] = '0'; // Ending character
        result[14] = '#'; // Ending character
        // Command-specific handling can be added here
        break;
    default:
        // Default handling for unknown commands
        break;
    }

    return result;
}

// Error checking function
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

    // Validate command-line arguments
    if (argc != 4)
    {
        printf("Usage: %s <name> <server address> <port number>\n", argv[0]);
        exit(1);
    }

    char *clientName = argv[1];
    int port = atoi(argv[3]);
    char *ip_addr = argv[2];

    printf("Connecting to %s on port %d\n", ip_addr, port);

    // Create the server address structure
    struct sockaddr_in serverAddr;
    bzero((char *)&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    // Convert IP address to binary form
    check(inet_pton(AF_INET, ip_addr, &serverAddr.sin_addr), "inet_pton error");

    // Create the TCP socket
    int sock = check(socket(AF_INET, SOCK_STREAM, 0), "socket() error");

    // Connect to the server
    check(connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)), "connect() error");

    printf("Connected to server.\n");

    // Send a predefined message to the server without user interaction
    char *input_buff = sendPackage(clientName, 'N');
    int snd = send(sock, input_buff, strlen(input_buff), 0);
    check(snd, "send() error");

    printf("Message sent: %s\n", input_buff);

    // Read the response from the server
    char output_buff[BUFFER_SIZE];
    ssize_t rc = recv(sock, output_buff, sizeof(output_buff), 0);
    if (rc < 0)
    {
        perror("recv() error");
        exit(1);
    }
    else if (rc == 0)
    {
        printf("Server closed connection :c\n");
    }
    else
    {
        printf("Received message from server:\n %.*s\n", (int)rc, output_buff);
    }

    // Close the socket
    close(sock);
    printf("Connection closed bye!\n");
    return 0;
}
