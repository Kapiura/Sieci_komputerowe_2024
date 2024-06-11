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

#define BLAD_GNIAZDA (-1)
#define MAKS_UZYTKOWNIKOW 10
#define MAKS_DANYCH 10000
#define ROZMIAR_KOMENDY 1000
#define PORT_POLACZENIA "7060"
#define ROZMIAR_NAZWY 8

typedef struct
{
    uint32_t dane[MAKS_DANYCH];
} DANE_OBLICZENIOWE;

typedef struct
{
    // Status połączenia, nazwa klienta, gniazdo klienta i gniazdo danych klienta
    int status;
    char nazwa[ROZMIAR_NAZWY + 1];
    char komenda;
    int klientFD;
    int licznik;

    // Numer portu dla danych serwera i deskryptor gniazda danych
    int status_portu_danych;
    int numer_portu_danych;
    int gniazdo_danychFD;
    int ilosc_wyslanych_danych;

    // Dane generowane przez serwer
    DANE_OBLICZENIOWE dane;
    char *wskaznik_na_dane_char;
} info_polaczenia;

int otworz_port_danych(info_polaczenia *polaczenie);
int oblicz_port(info_polaczenia *polaczenie);

void sprawdz(int kod_zwrotny, char *wiadomosc);
void obsluz_polaczenie(info_polaczenia *polaczenie, fd_set *aktualne_gniazda);
int akceptuj_nowe_polaczenie(int gniazdoFD);

int generuj_dane(int klientFD, DANE_OBLICZENIOWE *dane_obliczeniowe);
void wyslij_dane(info_polaczenia *polaczenie, fd_set *aktualne_gniazda);
int stworz_pakiet(char *pakiet, char opcja, info_polaczenia *polaczenie);
int wyslij_pakiet(int klientFD, char *pakiet);

char przetworz_pakiet_klienta(char *bufor_klienta);
void sprawdz_odpowiedz(char *bufor_klienta);

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        printf("./server <numer portu>\n");
        return 1;
    }

    info_polaczenia polaczenia[MAKS_UZYTKOWNIKOW];

    // Inicjalizacja informacji o połączeniach
    for (int i = 0; i < MAKS_UZYTKOWNIKOW; i++)
    {
        polaczenia[i].status = -1;
        polaczenia[i].status_portu_danych = -1;
        polaczenia[i].ilosc_wyslanych_danych = 0;
        polaczenia[i].komenda = ' ';
        polaczenia[i].licznik = 0;
    }

    int gniazdoFD, gniazdo_polaczenia, klientFD;

    // Utworzenie gniazda
    sprawdz((gniazdoFD = socket(AF_INET, SOCK_STREAM, 0)), "blad socketu()");

    struct sockaddr_in adres_gniazda;
    bzero(&adres_gniazda, sizeof(adres_gniazda));
    adres_gniazda.sin_family = AF_INET;
    adres_gniazda.sin_addr.s_addr = INADDR_ANY;
    adres_gniazda.sin_port = htons(atoi(argv[1]));

    // Powiązanie gniazda z adresem
    sprawdz(bind(gniazdoFD, (struct sockaddr *)&adres_gniazda, sizeof(adres_gniazda)), "blad bind()");

    // Słuchanie na gnieździe
    sprawdz(listen(gniazdoFD, MAKS_UZYTKOWNIKOW), "blad listen()");

    fd_set aktualne_gniazda, gotowe_gniazda;

    FD_ZERO(&aktualne_gniazda);
    FD_SET(gniazdoFD, &aktualne_gniazda);

    struct timeval czas = {5, 5};

    while (1)
    {
        gotowe_gniazda = aktualne_gniazda;
        sprawdz(select(FD_SETSIZE, &gotowe_gniazda, NULL, NULL, &czas), "blad select()");

        // Akceptowanie nowych połączeń
        if (FD_ISSET(gniazdoFD, &gotowe_gniazda))
        {
            klientFD = akceptuj_nowe_polaczenie(gniazdoFD);
            for (int i = 0; i < MAKS_UZYTKOWNIKOW; i++)
            {
                if (polaczenia[i].status == -1)
                {
                    polaczenia[i].klientFD = klientFD;
                    polaczenia[i].status = 1;
                    FD_SET(klientFD, &aktualne_gniazda);
                    break;
                }
            }
        }

        // Obsługa aktywnych połączeń
        for (int i = 0; i < MAKS_UZYTKOWNIKOW; i++)
        {
            if (FD_ISSET(polaczenia[i].klientFD, &gotowe_gniazda) && polaczenia[i].status == 1)
            {
                polaczenia[i].licznik++;
                obsluz_polaczenie(&polaczenia[i], &aktualne_gniazda);
            }
            if (polaczenia[i].status_portu_danych == 1)
            {
                wyslij_dane(&polaczenia[i], &aktualne_gniazda);
            }
        }

        // Wyświetlanie statusu klientów
        for (int i = 0; i < MAKS_UZYTKOWNIKOW; i++)
        {
            if (polaczenia[i].licznik != 0)
            {
                printf("klient: %d i licznik: %d\n", polaczenia[i].klientFD, polaczenia[i].licznik);
            }
        }
    }

    return 0;
}

// Sprawdzanie kodu powrotnego funkcji systemowych i wyświetlanie błędów
void sprawdz(int kod_zwrotny, char *wiadomosc)
{
    if (kod_zwrotny == BLAD_GNIAZDA)
    {
        perror(wiadomosc);
        exit(EXIT_FAILURE);
    }
}

// Akceptowanie nowego połączenia
int akceptuj_nowe_polaczenie(int gniazdoFD)
{
    int klientFD;
    struct sockaddr_in adres_klienta;
    socklen_t dlugosc = sizeof(adres_klienta);

    sprawdz(klientFD = accept(gniazdoFD, (struct sockaddr *)&adres_klienta, &dlugosc), "blad accept()");
    printf("Klient: %d polaczyl sie\n", klientFD);

    return klientFD;
}

// Obsługa połączenia z klientem
void obsluz_polaczenie(info_polaczenia *polaczenie, fd_set *aktualne_gniazda)
{
    int klientFD = polaczenie->klientFD;
    int komenda_przeczytana;
    char bufor_klienta[ROZMIAR_KOMENDY];
    char pakiet[ROZMIAR_KOMENDY];

    printf("Obslugiwany klient: %d\n", klientFD);

    // Odbieranie danych od klienta
    komenda_przeczytana = recv(klientFD, bufor_klienta, ROZMIAR_KOMENDY, 0);

    if (komenda_przeczytana > 0)
    {
        polaczenie->komenda = przetworz_pakiet_klienta(bufor_klienta);
        printf("Odebrano znak komendy: %c\n", polaczenie->komenda);
        printf("Odebrano pakiet od klienta: %s\n", bufor_klienta);
    }

    else if (komenda_przeczytana == 0)
    {
        printf("Brak nowych danych od tego klienta\n\n\n");
    }

    else if (komenda_przeczytana < 0)
    {
        printf("Blad przy odczycie komendy\n\n\n");
        exit(EXIT_FAILURE);
    }

    // Obsługa komend klienta
    switch (polaczenie->komenda)
    {
    case 'N':
        if (polaczenie->status_portu_danych == -1)
        {
            polaczenie->gniazdo_danychFD = otworz_port_danych(polaczenie);
            polaczenie->status_portu_danych = 1;
            FD_SET(polaczenie->gniazdo_danychFD, aktualne_gniazda);
            printf("Otworzono port polaczenia, ID portu: %d\n", polaczenie->gniazdo_danychFD);

            sprawdz(stworz_pakiet(pakiet, 'P', polaczenie), "blad tworzenia pakietu");
            sprawdz(wyslij_pakiet(klientFD, pakiet), "blad wysylania pakietu");

            printf("Wyslano pakiet do klienta: %d\n", klientFD);
            printf("Pakiet: %s\n", pakiet);

            polaczenie->gniazdo_danychFD = akceptuj_nowe_polaczenie(polaczenie->gniazdo_danychFD);

            DANE_OBLICZENIOWE dane;
            sprawdz(generuj_dane(klientFD, &dane), "blad tworzenia danych");
            polaczenie->dane = dane;
            polaczenie->wskaznik_na_dane_char = (char *)dane.dane;
            printf("Zakończono przetwarzanie danych\n");
        }
        break;

    case 'R':
        sprawdz_odpowiedz(bufor_klienta);
        sprawdz(stworz_pakiet(pakiet, 'D', polaczenie), "blad tworzenia pakietu");
        sprawdz(wyslij_pakiet(klientFD, pakiet), "blad wysylania pakietu");
        printf("Wyslano pakiet do klienta: %d\n", klientFD);
        printf("Pakiet: %s\n", pakiet);
        break;

    case 'P':
        FD_CLR(polaczenie->klientFD, aktualne_gniazda);
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

// Sprawdzanie odpowiedzi od klienta
void sprawdz_odpowiedz(char *bufor_klienta)
{
}

// Tworzenie pakietu do wysłania do klienta
int stworz_pakiet(char *pakiet, char opcja, info_polaczenia *polaczenie)
{
    bzero(pakiet, ROZMIAR_KOMENDY);

    switch (opcja)
    {
    case 'P':
        pakiet[0] = '@';
        for (int i = 1; i < 9; i++)
        {
            pakiet[i] = '0';
        }
        pakiet[9] = '!';
        pakiet[10] = opcja;
        pakiet[11] = ':';
        char port_str[5];
        sprintf(port_str, "%d", oblicz_port(polaczenie));
        for (int i = 0; i < strlen(port_str); i++)
        {
            pakiet[i + 12] = port_str[i];
        }
        pakiet[16] = '#';
        break;
    case 'X':
        break;
    case 'D':
        break;
    }
    return 1;
}

// Wysyłanie pakietu do klienta
int wyslij_pakiet(int klientFD, char *pakiet)
{
    sprawdz(write(klientFD, pakiet, ROZMIAR_KOMENDY), "blad wysylania pakietu");
    return 1;
}

// Przetwarzanie pakietu odebranego od klienta
char przetworz_pakiet_klienta(char *bufor_klienta)
{
    for (int i = 0; i < ROZMIAR_KOMENDY; i++)
    {
        if (bufor_klienta[i] == '!')
        {
            return bufor_klienta[i + 1];
        }
    }
    return 'Q';
}

// Wysyłanie danych do klienta
void wyslij_dane(info_polaczenia *polaczenie, fd_set *aktualne_gniazda)
{
    int ilosc_wyslanych_bajtow =
        send(polaczenie->gniazdo_danychFD, polaczenie->wskaznik_na_dane_char + polaczenie->ilosc_wyslanych_danych,
             MAKS_DANYCH * sizeof(uint32_t) - polaczenie->ilosc_wyslanych_danych, 0);

    // Sprawdzenie, czy dane zostały wysłane poprawnie
    if (ilosc_wyslanych_bajtow < 0)
    {
        perror("blad send()");
        close(polaczenie->gniazdo_danychFD);
        FD_CLR(polaczenie->gniazdo_danychFD, aktualne_gniazda);
        polaczenie->status_portu_danych = -1;
        return;
    }

    // Aktualizacja liczby wysłanych danych
    polaczenie->ilosc_wyslanych_danych += ilosc_wyslanych_bajtow;

    // Sprawdzenie, czy wszystkie dane zostały wysłane
    if (polaczenie->ilosc_wyslanych_danych >= MAKS_DANYCH * sizeof(uint32_t))
    {
        close(polaczenie->gniazdo_danychFD);
        FD_CLR(polaczenie->gniazdo_danychFD, aktualne_gniazda);
        polaczenie->status_portu_danych = -1;
        printf("Wysłano wszystkie dane do klienta %d\n", polaczenie->klientFD);
    }
}

// Funkcja do generowania danych
int generuj_dane(int klientFD, DANE_OBLICZENIOWE *dane_obliczeniowe)
{
    for (int i = 0; i < MAKS_DANYCH; i++)
    {
        dane_obliczeniowe->dane[i] = rand();
    }
    printf("Wygenerowano dane dla klienta %d\n", klientFD);
    return 0;
}

// Funkcja otwierająca port danych
int otworz_port_danych(info_polaczenia *polaczenie)
{
    int gniazdoFD = socket(AF_INET, SOCK_STREAM, 0);
    if (gniazdoFD == BLAD_GNIAZDA)
    {
        perror("blad socket()");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in adres_gniazda;
    bzero(&adres_gniazda, sizeof(adres_gniazda));
    adres_gniazda.sin_family = AF_INET;
    adres_gniazda.sin_addr.s_addr = INADDR_ANY;
    adres_gniazda.sin_port = htons(oblicz_port(polaczenie));

    if (bind(gniazdoFD, (struct sockaddr *)&adres_gniazda, sizeof(adres_gniazda)) == BLAD_GNIAZDA)
    {
        perror("blad bind()");
        exit(EXIT_FAILURE);
    }

    if (listen(gniazdoFD, 1) == BLAD_GNIAZDA)
    {
        perror("blad listen()");
        exit(EXIT_FAILURE);
    }

    return gniazdoFD;
}

// Funkcja obliczająca numer portu dla danych
int oblicz_port(info_polaczenia *polaczenie)
{
    return 7061 + (polaczenie - &polaczenie[0]);
}
