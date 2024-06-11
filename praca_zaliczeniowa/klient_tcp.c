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

// Definicja struktury do przechowywania komendy i numeru portu
typedef struct
{
    char port[4];
    char r;
} commandR;

// Funkcja sprawdzająca błędy i wypisująca komunikat, jeśli wystąpił błąd
int check(int exp, const char *mess)
{
    if (exp < 0)
    {
        perror(mess);
        exit(1);
    }
    return exp;
}

// Tablica do przechowywania histogramu bitów
uint32_t histogram[16];

// Funkcja czyszcząca histogram
void clear_histogram()
{
    bzero(histogram, sizeof(histogram));
}

// Funkcja tworząca pakiet do wysłania
int sendPackage(char *package, char *nameBuff, char command, uint32_t *bitCounter)
{
    size_t nameLen = strlen(nameBuff);
    package[0] = '@';

    // Dodawanie nazwy klienta do pakietu, wypełnianie brakujących miejsc znakami '_'
    if (nameLen < 8)
    {
        char paddedName[9] = {0};
        strncpy(paddedName, nameBuff, nameLen);
        memset(paddedName + nameLen, '_', 8 - nameLen);
        strncpy(&package[1], paddedName, 8);
    }
    else
    {
        strncpy(&package[1], nameBuff, 8);
    }

    // Dodawanie stałych wartości do pakietu
    package[9] = '0'; // Zapewnienie, że '0' jest na indeksie 9
    package[10] = '!';
    package[11] = command;
    package[12] = ':';
    package[999] = '#';

    // Obsługa specyficznych komend
    switch (command)
    {
    case 'R':
        for (int i = 0; i < 16; i++)
        {
            package[i + 13] = bitCounter[i];
        }
        break;

    case 'P':
    case 'D':
    case 'X':
        break;

    default:
        break;
    }

    return 0;
}

// Funkcja zwracająca bit z określonej pozycji
int getBit(int position, int number)
{
    return (number >> position) & 1;
}

// Funkcja przetwarzająca komendę 'P'
void commandP(char *package, commandR *r)
{
    for (int i = 12; package[i] != '#'; i++)
    {
        r->port[i - 12] = package[i];
    }
    r->r = package[10];
}

// Funkcja przetwarzająca dane przychodzące od serwera i aktualizująca licznik bitów
void connP(char *buffer, uint32_t *bit_count)
{
    uint32_t number = *((uint32_t *)&buffer[0]);
    printf("%d\n", number);
    for (int i = 15; i >= 0; i--)
    {
        bit_count[i] += getBit(i, number) == 1 ? 1 : 0;
        printf("%d", getBit(i, number));
    }
    printf("\nend\n");
}

// Funkcja obsługująca połączenie z serwerem i odbierająca dane
void handle(int sk, uint32_t *bitCounter)
{
    int bytes_read = 1;
    int cumulative_read = 0;
    int counter = 0;
    char read_buffer[4];

    while (bytes_read > 0)
    {
        // Odbieranie danych z gniazda
        bytes_read = recv(sk, read_buffer, 4, 0);
        if (bytes_read < 0)
        {
            perror("recv() error");
            exit(EXIT_FAILURE);
        }

        cumulative_read += bytes_read;
        counter++;

        printf("Liczba bajtów odczytanych w tej chwili: %d\n", bytes_read);
        printf("i: %d\n", counter);

        // Przerwanie, jeśli odczytano wystarczającą ilość danych
        if (cumulative_read >= MAX_DATA * 4)
        {
            break;
        }

        // Przetwarzanie odebranych danych
        connP(read_buffer, bitCounter);
    }

    printf("\n\n");
    for (int i = 0; i < 16; i++)
    {
        printf("Licznik: %d\n Liczba: %d\n", i, bitCounter[i]);
    }

    printf("\nDane z serwera: ");
}

// Funkcja main - punkt wejścia programu
int main(int argc, char *argv[])
{
    // Sprawdzanie poprawności argumentów wywołania
    if (argc != 4)
    {
        printf("Usage: %s <name> <server address> <port number>\n", argv[0]);
        exit(1);
    }

    // Inicjalizacja zmiennych
    char *clientName = argv[1];
    int port = atoi(argv[3]);
    char *ip_addr = argv[2];

    // Tworzenie gniazda
    int sock = check(socket(AF_INET, SOCK_STREAM, 0), "socket() error");

    printf("Łączenie się z %s na porcie %d\n", ip_addr, port);

    // Tworzenie struktury adresu serwera
    struct sockaddr_in serverAddr;
    bzero((char *)&serverAddr, sizeof(serverAddr));
    serverAddr.sin_addr.s_addr = inet_addr(ip_addr);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    // Łączenie się z serwerem
    check(connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)), "connect() error");

    printf("Połączono z serwerem.\n");

    // Wysyłanie predefiniowanej wiadomości do serwera bez interakcji z użytkownikiem
    int conn;
    char serverBuff[BUFFER_SIZE];
    commandR cr;

    char package[BUFFER_SIZE];
    memset(package, '0', sizeof(package));

    // Wysyłanie pakietu 'N' (nowe dane) do serwera
    check(sendPackage(package, clientName, 'N', NULL), "sendPackage() error");
    printf("Pakiet: %s\n", package);
    check(write(sock, package, BUFFER_SIZE), "write() error");
    read(sock, serverBuff, BUFFER_SIZE);

    // Przetwarzanie odpowiedzi serwera
    commandP(serverBuff, &cr);
    printf("Numer portu od serwera: %s\nKomenda od serwera: %c\n", cr.port, cr.r);

    while (1)
    {
        for (;;)
        {
            switch (cr.r)
            {
            case 'P':
                check(sock = socket(AF_INET, SOCK_STREAM, 0), "socket() error");

                struct sockaddr_in serverAddr;
                bzero((char *)&serverAddr, sizeof(serverAddr));
                serverAddr.sin_addr.s_addr = inet_addr(ip_addr);
                serverAddr.sin_family = AF_INET;
                serverAddr.sin_port = htons(atoi(cr.port));

                uint32_t bitCounter[16];
                for (int i = 0; i < 16; i++)
                {
                    bitCounter[i] = 0;
                }

                // Obsługa połączenia z serwerem i odbieranie danych
                handle(sock, bitCounter);
                close(sock);
                printf("Pomyślnie odebrano dane z serwera\n");

                // Wysyłanie wyników obliczeń do serwera
                sendPackage(package, clientName, 'R', bitCounter);
                check(write(sock, package, BUFFER_SIZE), "write() error");

                read(sock, serverBuff, BUFFER_SIZE);
                commandP(serverBuff, &cr);
                break;

            case 'X':
                close(sock);
                exit(EXIT_SUCCESS);

            case 'D':
                goto ex;

            default:
                break;
            }
        }
    ex:;
    }

    return 0;
}
