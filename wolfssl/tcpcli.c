
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

int main(int argc, char *argv[])
{
    // zmienne gniazdka oraz dlugosci adresu
    int sock;
    int addr_length;
    // pobieranie adresu oraz portu z linii komend
    if (argc < 3)
    {
        printf("Usage: <address> <port>\n");
        exit(1);
    }

    // ustawienie portu
    int portno = atoi(argv[2]);

    // inicjalizacja struktury serwera => uzywana do przychowywania adresu IP oraz numeru portu z ktorymi chcemy
    // nawiazac polaczenie
    struct sockaddr_in serwer;
    bzero((char *)&serwer, sizeof(serwer));
    serwer.sin_family = AF_INET;
    serwer.sin_port = htons(portno);

    // uzyskanie informacji na temat adresu IP serwera z ktorym chcemy sie polaczyc za pomoca nazwy
    struct hostent *host;
    // nazwa serwera jako argument z linii komend
    char dnsbuffer[255];
    strncpy(dnsbuffer, argv[1], sizeof(dnsbuffer) - 1);
    host = gethostbyname(dnsbuffer);
    // sprawdzenie czy host istnieje
    // jesli tak wyswietlamy jego adres IP
    // kopiowanie zwrocenego adresu IP
    if (host != NULL)
    {
        printf("Ip adress of %s : ", host->h_name);
        printf("%s\n\n", inet_ntoa(*(struct in_addr *)host->h_addr));
        memcpy(&serwer.sin_addr, host->h_addr, host->h_length);
    }
    // jesli nie ma takiego
    else
    {
        perror("gethostbyname() ERROR");
        exit(2);
    }

    // otwierane socketu
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (!sock)
    {
        printf("socket() error\n");
        exit(3);
    }

    // polaczenie gniazda z serwerem
    int con = connect(sock, (struct sockaddr *)&serwer, sizeof(serwer));
    if (!con)
    {
        printf("connect() error\n");
        exit(4);
    }

    // init wolfa
    wolfSSL_Init();

    // tworzenie polaczenia ssl dla klienta
    ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method());
    if (ctx == NULL)
    {
        printf("error during creating wolfssl_ctx\n");
        exit(5);
    }

    // ustawienie metody weryfikacji certyfikatow
    wolfSSL_CTX_set_verify(ctx, WOLFSSL_VERIFY_NONE, NULL);

    // tworzy strukture ssl
    ssl = wolfSSL_new(ctx);
    if (ssl == NULL)
    {
        printf("error during creating ssl structure\n");
        exit(6);
    }

    // ustawienie deskryptora pliku dla ssl
    wolfSSL_set_fd(ssl, sock);

    // polaczaenie ssl klienta z serwerem
    if (wolfSSL_connect(ssl) != WOLFSSL_SUCCESS)
    {
        printf("ssl connect error\n");
        exit(7);
    }

    // tworzenie requesta HTTP 1.1 w podany sposob na stronie
    char httpreq[] = "HEAD /index.html HTTP/1.1\r\nHost: google.com\r\nUser-Agent: Mozilla/5.0 (X11; "
                     "Ubuntu;)\r\nAccept-Language: en-US\r\nAccept-Encoding: gzip\r\n\r\n";

    // wysylanie zapytania do serwera http
    int httpWolfReq = wolfSSL_write(ssl, httpreq, sizeof(httpreq) - 1);
    if (httpWolfReq)
    {
        printf("wolfSSL_write() error\n");
        exit(8);
    }

    // tablica do przechopwywania danych przeslanych przez serwer
    char buffer[4096];
    // liczba odczytanych bajtow
    int ret = wolfSSL_read(ssl, buffer, sizeof(buffer) - 1);
    if (ret > 0)
    {
        buffer[ret] = '\0';
        printf("%s\n", buffer);
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
