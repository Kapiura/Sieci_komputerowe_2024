#include <arpa/inet.h>
#include <bits/types/struct_iovec.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define SOCKET_ERROR (-1)
#define COMMAND_SIZE 1000
#define MAX_DATA 10000
#define NAME_LENGTH 8

typedef struct
{
    uint32_t data[MAX_DATA];
} CALCDATA;

typedef struct
{
    char port_number[4];
    char resp;
} command_resp;

void check(int return_code, char *message);
void run_client(int clientFD, char *name, char *server_address);
void process_conn(char *buffer, uint32_t *bit_count);
void handle_conn(int connFD, uint32_t *bit_count);
void process_command(char *package, command_resp *resp);
int create_package(char *package, char *name, char option, uint32_t *bit_count);
void connect_with_server(int clientFD, char *server_address, char *port_number);

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("./client <name> <server adress> <port number>\n");
        return 1;
    }

    int clientFD;

    check(clientFD = socket(AF_INET, SOCK_STREAM, 0), "socket() error");
    connect_with_server(clientFD, argv[2], argv[3]);

    while (1)
    {
        run_client(clientFD, argv[1], argv[2]);
    }
    return 0;
}

void run_client(int clientFD, char *name, char *server_address)
{
    int connFD;
    char server_package[COMMAND_SIZE];
    //  CALCDATA conn;
    command_resp resp;

    char package[COMMAND_SIZE];
    memset(package, '0', sizeof(package));

    check(create_package(package, name, 'N', NULL), "Name is to long, max 8 characters\n");
    printf("package: %s\n", package);

    check(write(clientFD, package, COMMAND_SIZE), "send() error");
    read(clientFD, server_package, COMMAND_SIZE);

    process_command(server_package, &resp);
    printf("Port number form server: %s, command from sever: %c\n", resp.port_number, resp.resp);
    /*read(clientFD, &conn, sizeof(conn));

    resp = process_conn(&conn);

  */
    while (1)
    {
        switch (resp.resp)
        {
        case 'P':
            check(connFD = socket(AF_INET, SOCK_STREAM, 0), "connFD socket() error");
            connect_with_server(connFD, server_address, resp.port_number);

            uint32_t bit_count[16];
            for (int i = 0; i < 16; i++)
            {
                bit_count[i] = 0;
            }

            handle_conn(connFD, bit_count);
            close(connFD);
            printf("Successfully got conn data from server\n");

            create_package(package, name, 'R', bit_count);
            check(write(clientFD, package, COMMAND_SIZE), "send() error");

            read(clientFD, server_package, COMMAND_SIZE);
            process_command(server_package, &resp);
            break;

        case 'X':
            close(clientFD);
            exit(EXIT_SUCCESS);

        case 'D':
            goto exit_loop;

        default:
            break;
        }
    }
exit_loop:;
}

void process_command(char *package, command_resp *resp)
{
    for (int i = 12; package[i] != '#'; i++)
    {
        resp->port_number[i - 12] = package[i];
    }
    resp->resp = package[10];
}

int get_bit(int position, int number)
{
    return (number >> position) & 1;
}

void process_conn(char *buffer, uint32_t *bit_count)
{
    uint32_t number = *((uint32_t *)&buffer[0]);
    printf("%d   :)\n", number);
    for (int i = 15; i >= 0; i--)
    {
        bit_count[i] += get_bit(i, number) == 1 ? 1 : 0;
        printf("%d", get_bit(i, number));
    }
    printf("\n\nThe end :)\n");
}

int create_package(char *package, char *name, char option, uint32_t *bit_count)
{
    if (strlen(name) > 8)
    {
        return -1;
    }

    package[0] = '@';
    for (int i = 0; i < NAME_LENGTH; i++)
    {
        if (i >= strlen(name))
        {
            package[i + 1] = '_';
            continue;
        }
        package[i + 1] = name[i];
    }
    package[NAME_LENGTH + 1] = '0';
    package[NAME_LENGTH + 2] = '!';
    package[NAME_LENGTH + 3] = option;
    package[NAME_LENGTH + 4] = ':';
    package[NAME_LENGTH + 5] = '#';

    switch (option)
    {
    case 'R':
        for (int i = 0; i < 16; i++)
        {
            package[i + 13] = bit_count[i];
        }
        break;

    case 'P':
        break;
    case 'D':
        break;
    case 'X':
        break;
    default:
        break;
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

void connect_with_server(int clientFD, char *server_address, char *port_number)
{
    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(server_address);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port_number));
    //  bzero(&server_address, sizeof(server_address));

    check(connect(clientFD, (struct sockaddr *)&server, sizeof(server)), "connect() error");
}

void handle_conn(int connFD, uint32_t *bit_count)
{
    int bytes_read = 1;
    int cumulative_read = 0;
    int counter = 0;
    char read_buffer[4];

    while (bytes_read > 0)
    {
        bytes_read = recv(connFD, read_buffer, 4, 0);
        cumulative_read += bytes_read;
        counter++;

        printf("Amount of bytes read in this step: %d\n", bytes_read);
        printf("Loop counter: %d\n", counter);

        if (cumulative_read >= MAX_DATA * 4)
        {
            break;
        }

        if (bytes_read < 0)
        {
            exit(EXIT_FAILURE);
        }
        process_conn(read_buffer, bit_count);
    }

    printf("\n\n");
    for (int i = 0; i < 16; i++)
    {
        printf("counter: %d, number: %d\n", i, bit_count[i]);
    }

    printf("\n\nGot data from server: ");
}
