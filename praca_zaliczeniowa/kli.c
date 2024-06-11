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
#define MAX_DATA 10000
#define NAME_LENGTH 8

typedef struct
{
    uint32_t data[MAX_DATA];
} CALCDATA;

typedef struct
{
    char port[4];
    char r;
} commandR;

// Funkcja do sprawdzania błędów
int check(int exp, const char *mess)
{
    if (exp < 0)
    {
        perror(mess);
        exit(1);
    }
    return exp;
}

// Przetwarzanie komendy otrzymanej w pakiecie
void process_command(char *package, commandR *r)
{
    // Pobieranie portu z pakietu
    for (int i = 12; package[i] != '#'; i++)
    {
        r->port[i - 12] = package[i];
    }
    // Pobieranie znaku komendy
    r->r = package[10];
}

// Pobieranie wartości bitu na określonej pozycji
int get_bit(int position, int number)
{
    return (number >> position) & 1;
}

// Tworzenie pakietu do wysłania
int create_package(char *package, char *nameBuff, char command, uint32_t *bitCounter)
{
    if (strlen(nameBuff) > 8)
    {
        return -1;
    }

    package[0] = '@';
    for (int i = 0; i < NAME_LENGTH; i++)
    {
        if (i >= strlen(nameBuff))
        {
            package[i + 1] = '_';
            continue;
        }
        package[i + 1] = nameBuff[i];
    }
    package[9] = '0';
    package[10] = '!';
    package[11] = command;
    package[12] = ':';
    package[13] = '#';

    switch (command)
    {
    case 'R':
        // Dodawanie licznika bitów do pakietu
        for (int i = 0; i < 16; i++)
        {
            package[i + 13] = bitCounter[i];
        }
        break;
    case 'P':
    case 'D':
    case 'X':
    default:
        break;
    }
    return 0;
}

// Funkcja do wysyłania pakietu do serwera
char *sendPackage(char *nameBuff, char command)
{
    static char result[BUFFER_SIZE];
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
    result[9] = '0'; // Upewnienie się, że '0' jest na indeksie 9
    result[10] = '!';
    result[11] = command;
    result[12] = ':';

    // Obsługa konkretnych komend
    switch (command)
    {
    case 'N':
        result[13] = '0'; // Znak końca
        result[14] = '#'; // Znak końca
        break;
    default:
        break;
    }

    return result;
}

// Funkcja do łączenia się z serwerem
void connectServer(int sock, char *ip, char *port)
{
    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port));
    check(connect(sock, (struct sockaddr *)&server, sizeof(server)), "connect() error");
}

// Przetwarzanie danych z połączenia
void process_conn(char *buffer, uint32_t *bitCounter)
{
    uint32_t number = *((uint32_t *)&buffer[0]);
    printf("%d\n", number);
    for (int i = 15; i >= 0; i--)
    {
        bitCounter[i] += get_bit(i, number) == 1 ? 1 : 0;
        printf("%d", get_bit(i, number));
    }
    printf("\n\nThe end\n");
}

// Obsługa połączenia
void handleConnection(int connf, uint32_t *bitCounter)
{
    int bytes_read = 1;
    int cumulative_read = 0;
    int counter = 0;
    char read_buffer[4];

    while (bytes_read > 0)
    {
        bytes_read = recv(connf, read_buffer, 4, 0);
        cumulative_read += bytes_read;
        counter++;

        printf("Amount of bytes read: %d\n", bytes_read);
        printf("i : %d\n", counter);

        if (cumulative_read >= MAX_DATA * 4)
        {
            break;
        }

        if (bytes_read < 0)
        {
            exit(EXIT_FAILURE);
        }
        process_conn(read_buffer, bitCounter);
    }

    printf("\n\n");
    for (int i = 0; i < 16; i++)
    {
        printf("counter: %d, number: %d\n", i, bitCounter[i]);
    }

    printf("\n\ndata from server: ");
}

// Funkcja główna klienta
int main(int argc, char *argv[])
{
    int essa = 1;

    // Sprawdzanie poprawności argumentów
    if (argc != 4)
    {
        printf("Usage: %s <name> <server address> <port number>\n", argv[0]);
        exit(1);
    }

    char *clientName = argv[1];
    char *port = argv[3];
    char *ip_addr = argv[2];

    int sock;
    sock = check(socket(AF_INET, SOCK_STREAM, 0), "socket() error");

    printf("Connecting to %s on port %s\n", ip_addr, port);

    // Tworzenie struktury adresu serwera
    connectServer(sock, ip_addr, port);

    printf("Connected to server.\n");

    int connf;
    char server_package[BUFFER_SIZE];
    commandR cr;

    char package[BUFFER_SIZE];
    memset(package, '0', sizeof(package));

    check(create_package(package, clientName, 'N', NULL), "error()\n");
    printf("Pakiet: %s", package);

    check(write(sock, package, BUFFER_SIZE), "write() error\n");
    read(sock, server_package, BUFFER_SIZE);

    process_command(server_package, &cr);

    // Wysłanie pakietu zdefiniowanego wcześniej bez interakcji użytkownika
    char *input_buff = sendPackage(clientName, 'N');
    int snd = send(sock, input_buff, strlen(input_buff), 0);
    check(snd, "send() error");

    printf("Message sent: %s\n", input_buff);

    while (essa)
    {
        switch (cr.r)
        {
        case 'P':
            check(connf = socket(AF_INET, SOCK_STREAM, 0), "connf socket() error");
            connectServer(connf, ip_addr, cr.port);

            uint32_t bit_count[16] = {0};

            handleConnection(connf, bit_count);
            close(connf);
            printf("Successfully got conn data from server\n");

            create_package(package, clientName, 'R', bit_count);
            check(write(sock, package, BUFFER_SIZE), "send() error");

            read(sock, server_package, BUFFER_SIZE);
            process_command(server_package, &cr);
            break;

        case 'X':
            close(sock);
            exit(EXIT_SUCCESS);

        case 'D':
            essa = 0;
            printf("Koniec klienta\n");
            exit(2);

        default:
            break;
        }
    }
    return 0;
}
