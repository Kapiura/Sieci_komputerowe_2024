
#include "wolfssl/options.h"
#include "wolfssl/ssl.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// zmienne globalne wymagane do dzialania programu
// struktury ssl z biblioteki wolfssl
WOLFSSL_CTX *ctx;
WOLFSSL *ssl;

WOLFSSL_CTX *ctx;
WOLFSSL *ssl;
int sock;
int addr_length;

int main(int argc, char *argv[])
{
    // zmienne gniazdka oraz dlugosci adresu
    // pobieranie adresu oraz portu z linii komend
    if (argc < 3)
    {
        printf("Usage: <address> <port>\n");
        exit(1);
    }
    // pobranie adresu oraz portu z linii komend - parametry przy uruchamianiu programu
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <address> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // ustawienie portu
    int portno = atoi(argv[2]);

    // inicjalizacja struktury, uzywanej do przechowywania adresu IP oraz numeru portu
    // z ktorym chcemy nawiazac polaczenie
    struct sockaddr_in serwer;
    bzero((char *)&serwer, sizeof(serwer));
    serwer.sin_family = AF_INET;
    serwer.sin_port = htons(portno);

    // uzyskanie informacji na temat adresu IP serwera
    // za pomoca funkcji
    struct hostent *host;
    char dnsbuffer[255];
    strncpy(dnsbuffer, argv[1], sizeof(dnsbuffer) - 1);
    host = gethostbyname(dnsbuffer);
    // sprawdzanie hosta czy taki sobie isntieje uwu
    // oraz wyprintowanie adresu ip
    // kopoiowanie adresu ip zwroconego czyli tego wyswieltonego
    if (host != NULL)
    {
        printf("\nIP address of %s is: ", host->h_name);
        printf("%s\n\n", inet_ntoa(*(struct in_addr *)host->h_addr));
        memcpy(&serwer.sin_addr, host->h_addr, host->h_length);
    }
    else
    {
        perror("gethostbyname() ERROR");
        exit(-16);
    }

    // otwieranie gniazda
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket() ERROR");
        exit(2);
    }

    // oplaczenie gniazda z serwerem
    if (connect(sock, (struct sockaddr *)&serwer, sizeof(serwer)) < 0)
    {
        perror("connect() ERROR");
        exit(3);
    }

    // inicjacja wolfa
    wolfSSL_Init();

    // Tworzenie polaczenia ssl dla klienta
    ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method());
    if (ctx == NULL)
    {
        fprintf(stderr, "Error creating WOLFSSL_CTX\n");
        exit(EXIT_FAILURE);
    }

    // ustawienie metody weryfikacji certyfikatow
    wolfSSL_CTX_set_verify(ctx, WOLFSSL_VERIFY_NONE, NULL);

    // tworzy strukture ssl
    ssl = wolfSSL_new(ctx);
    if (ssl == NULL)
    {
        fprintf(stderr, "Error creating SSL structure\n");
        exit(EXIT_FAILURE);
    }

    // ustawienie deskryptora pliku dla ssl
    wolfSSL_set_fd(ssl, sock);

    // polaczaenie ssl
    if (wolfSSL_connect(ssl) != WOLFSSL_SUCCESS)
    {
        fprintf(stderr, "SSL connect error\n");
        exit(EXIT_FAILURE);
    }

    char httpreq[] = "HEAD /index.html HTTP/1.1\r\nHost: google.com\r\nUser-Agent: Mozilla/5.0 (X11; "
                     "Ubuntu;)\r\nAccept-Language: en-US\r\nAccept-Encoding: gzip\r\n\r\n";

    // wysylanie zapytania do serwera http
    if (wolfSSL_write(ssl, httpreq, sizeof(httpreq) - 1) < 0)
    {
        perror("wolfSSL_write() ERROR");
        exit(EXIT_FAILURE);
    }

    // czekanie 5 sekund
    sleep(5);

    // tablica do przechopwywania danych
    char wwwbuffer[4096];
    // liczba odczytanych bajtow
    int ret = wolfSSL_read(ssl, wwwbuffer, sizeof(wwwbuffer) - 1);
    if (ret > 0)
    {
        wwwbuffer[ret] = '\0';
        printf("%s\n", wwwbuffer);
    }

    // zamkykanie polaczania ssl
    wolfSSL_shutdown(ssl);
    // zwalnianie pamieci zajmowanej przez strukture ssl
    wolfSSL_free(ssl);
    wolfSSL_CTX_free(ctx);
    // czyszczenie zasobow uzywanych przez biblioteke
    wolfSSL_Cleanup();

    // zamkyaknie gniazda
    shutdown(sock, SHUT_RDWR);
    close(sock);

    return 0;
}
