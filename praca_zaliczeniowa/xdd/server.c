#include <arpa/inet.h>
#include <bits/types/struct_timeval.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define SOCKET_ERROR (-1)
#define MAX_USERS 10
#define MAX_DATA 10000
#define COMMAND_SIZE 1000
#define CONN_PORT "7060"
#define NAME_SIZE 8

typedef struct
{
    uint32_t data[MAX_DATA];
} CALCDATA;

typedef struct
{
    // Connection status, client name, client socket and client data socket
    int status;
    char name[NAME_SIZE + 1];
    char command;
    int clientFD;
    int counter;

    // Port number for server generated data and data socket file descriptor
    int data_port_status;
    int data_port_number;
    int dataFD;
    int amount_of_data_sent;

    // Server generated data
    CALCDATA data;
    char *char_data_pointer;
} connection_info;

int open_data_port(connection_info *connection);
int calc_port(connection_info *connection);

void check(int return_code, char *message);
void handle_connection(connection_info *connection, fd_set *current_sockets);
int accept_new_connection(int socketFD);

int create_data(int clientFD, CALCDATA *cdata);
void send_data(connection_info *connection, fd_set *current_sockets);
int create_package(char *package, char option, connection_info *connection);
int send_package(int clientFD, char *package);

char process_client_package(char *client_buffer);
void validate_answer(char *client_buffer);

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        printf("./server <port number>\n");
        return 1;
    }

    connection_info connections[MAX_USERS];

    for (int i = 0; i < MAX_USERS; i++)
    {
        connections[i].status = -1;
        connections[i].data_port_status = -1;
        connections[i].amount_of_data_sent = 0;
        connections[i].command = ' ';
        connections[i].counter = 0;
    }

    int socketFD, connFD, clientFD;

    check((socketFD = socket(AF_INET, SOCK_STREAM, 0)), "socket() error");

    struct sockaddr_in socket_adddress;
    bzero(&socket_adddress, sizeof(socket_adddress));
    socket_adddress.sin_family = AF_INET;
    socket_adddress.sin_addr.s_addr = INADDR_ANY;
    socket_adddress.sin_port = htons(atoi(argv[1]));

    check(bind(socketFD, (struct sockaddr *)&socket_adddress, sizeof(socket_adddress)), "bind() error");

    check(listen(socketFD, MAX_USERS), "listen() error");

    fd_set current_sockets, ready_sockets;

    FD_ZERO(&current_sockets);
    FD_SET(socketFD, &current_sockets);

    struct timeval time = {5, 5};

    while (1)
    {
        ready_sockets = current_sockets;
        check(select(FD_SETSIZE, &ready_sockets, NULL, NULL, &time), "select() error");

        if (FD_ISSET(socketFD, &ready_sockets))
        {
            clientFD = accept_new_connection(socketFD);
            for (int i = 0; i < MAX_USERS; i++)
            {
                if (connections[i].status == -1)
                {
                    connections[i].clientFD = clientFD;
                    connections[i].status = 1;
                    FD_SET(clientFD, &current_sockets);
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_USERS; i++)
        {
            if (FD_ISSET(connections[i].clientFD, &ready_sockets) && connections[i].status == 1)
            {
                connections[i].counter++;
                handle_connection(&connections[i], &current_sockets);
            }
            if (connections[i].data_port_status == 1)
            {
                send_data(&connections[i], &current_sockets);
            }
        }
        for (int i = 0; i < MAX_USERS; i++)
        {
            if (connections[i].counter != 0)
            {
                printf("client: %d and counter: %d\n", connections[i].clientFD, connections[i].counter);
            }
        }
    }

    return 0;
}

void check(int return_code, char *message)
{
    if (return_code == SOCKET_ERROR)
    {
        perror(message);
        exit(EXIT_FAILURE);
    }
}

int accept_new_connection(int socketFD)
{
    int clientFD;
    struct sockaddr_in client_address;
    socklen_t length = sizeof(client_address);

    check(clientFD = accept(socketFD, (struct sockaddr *)&client_address, &length), "accept() error");
    printf("Client: %d connected\n", clientFD);

    return clientFD;
}

void handle_connection(connection_info *connection, fd_set *current_sockets)
{
    int clientFD = connection->clientFD;
    int command_read;
    char client_buffer[COMMAND_SIZE];
    char package[COMMAND_SIZE];

    printf("Client handled: %d\n", clientFD);

    // TODO: Change all reads and writes with recv and send
    command_read = recv(clientFD, client_buffer, COMMAND_SIZE, 0);

    if (command_read > 0)
    {
        connection->command = process_client_package(client_buffer);
        printf("Package character: %c\n", connection->command);
        printf("COMMAND package got from user: %s\n", client_buffer);
    }

    else if (command_read == 0)
    {
        printf("Nothing new from this client\n\n\n");
    }

    else if (command_read < 0)
    {
        printf("ERROR command reading\n\n\n");
        exit(EXIT_FAILURE);
    }

    switch (connection->command)
    {
    case 'N':
        if (connection->data_port_status == -1)
        {
            connection->dataFD = open_data_port(connection);
            connection->data_port_status = 1;
            FD_SET(connection->dataFD, current_sockets);
            printf("Opened conn_port, conn_port id: %d\n", connection->dataFD);

            check(create_package(package, 'P', connection), "create_package() error");
            check(send_package(clientFD, package), "send_package() error");

            printf("Sent B1 package to client: %d\n", clientFD);
            printf("package: %s\n", package);

            connection->dataFD = accept_new_connection(connection->dataFD);

            CALCDATA data;
            check(create_data(clientFD, &data), "create_data() error");
            connection->data = data;
            connection->char_data_pointer = (char *)data.data;
            printf("DOne everythin here :)\n");
        }
        break;

    case 'R':
        validate_answer(client_buffer);
        check(create_package(package, 'D', connection), "create_package() error");
        check(send_package(clientFD, package), "send_package() error");
        printf("Sent B3 package to client: %d\n", clientFD);
        printf("package: %s\n", package);
        break;

    case 'P':
        FD_CLR(connection->clientFD, current_sockets);
        break;
    case 'D':
        break;
    case 'E':
        break;
    case 'X':
        break;
    default:
        break;
    }
}

void validate_answer(char *client_buffer)
{
}

int create_package(char *package, char option, connection_info *connection)
{
    // char port_num[] = "7001";
    bzero(package, COMMAND_SIZE);

    switch (option)
    {
    case 'P':
        package[0] = '@';
        for (int i = 1; i < 9; i++)
        {
            package[i] = '0';
        }
        package[9] = '!';
        package[10] = option;
        package[11] = ':';
        char port_str[5];
        sprintf(port_str, "%d", calc_port(connection));
        for (int i = 0; i < strlen(port_str); i++)
        {
            package[i + 12] = port_str[i];
        }
        package[16] = '#';
        break;
    case 'X':
        break;
    case 'D':
        break;
    }
    return 1;
}

int send_package(int clientFD, char *package)
{
    check(write(clientFD, package, COMMAND_SIZE), "write() error");
    return 1;
}

char process_client_package(char *client_buffer)
{
    for (int i = 0; i < COMMAND_SIZE; i++)
    {
        if (client_buffer[i] == '!')
        {
            return client_buffer[i + 1];
        }
    }
    return 'Q';
}

void send_data(connection_info *connection, fd_set *current_sockets)
{
    printf("Sending data\n");
    if (connection->amount_of_data_sent >= 4 * MAX_DATA)
    {
        connection->data_port_status = -2;
        FD_CLR(connection->dataFD, current_sockets);
        close(connection->dataFD);
        return;
    }

    // char* char_data_pointer = (char*) connection->data.data;
    int write_buffer = 0;

    check(write_buffer = send(connection->dataFD, connection->char_data_pointer, 1000, MSG_NOSIGNAL),
          "write() error for conn data");
    connection->amount_of_data_sent += write_buffer;

    printf("Cumulative amount of data written to buffer: %d\n", connection->amount_of_data_sent);
    printf("Amount of written in this step: %d\n", write_buffer);

    if (write_buffer < 0)
    {
        printf("ERROR occured when sending data :(");
    }
}

int create_data(int idx, CALCDATA *cdata)
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
        return 0;
    }
    return -1;
}

int open_data_port(connection_info *connection)
{
    int connFD;
    check((connFD = socket(AF_INET, SOCK_STREAM, 0)), "socket() error conn_port");

    struct sockaddr_in socket_adddress;
    bzero(&socket_adddress, sizeof(socket_adddress));
    socket_adddress.sin_family = AF_INET;
    socket_adddress.sin_addr.s_addr = INADDR_ANY;
    socket_adddress.sin_port = htons(calc_port(connection));

    check(bind(connFD, (struct sockaddr *)&socket_adddress, sizeof(socket_adddress)), "bind() error conn_port");

    check(listen(connFD, MAX_USERS), "listen() error conn_port");

    return connFD;
}

int calc_port(connection_info *connection)
{
    return atoi(CONN_PORT) + connection->clientFD + 1;
}
