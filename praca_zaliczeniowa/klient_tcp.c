/*
 * KLIENT TCP
 * ODBIERA DANE OD SERWERA
 * WYKONUJE OBLICZENIA
 * WYSYLA WYNIKI DO SERWERA
 *
 * KLIENT URUCHAMIANY Z PARAMETRAMI
 * <nazwa> <adres serwera> <nr portu>
 *
 * ZA POMOCA SKYRPTU W DOCELOWYM TESCIE
 * URUCHAMIAMY DO 10 KLIENTOW
 *
 * KLIENT LACZY SIE Z SERWEREM ZA POMOCA
 * ADRESU I PORTU
 *
 * COMMAND
 *
 * PO POLACZENIU KLIENT WYSYLA DO SERWERA
 * PAKIET A1 Z NAZWA PODANA JAKO PARAMETR
 * I KOMENDA N => PROSBA O NOWE DANE
 *
 * KLIENT PO DOSTANIU KOMENDY P OTWIERA POLACZENIE DATA
 * NA WSKAZANY NUMER PORITU I POBIERA DANE
 * WYKONUJE OBLICZENIA
 * ODSYLA WYIKI W PAKIECIE A2 Z KOMENDA R
 * JESLI WYSTAPI BLAD, KLIENT WYSYLA DO SERWERA PAKIET A3
 *
 * Z KOMENDA E
 *
 * PO OTRZYMANIU KOMENDY X KONCZY PRACE KLIENT
 *
 * KLIENT PO OTRZYMANIU D KONCZY POALCZAENIE COMMAND I ROZOPOCZYNA PRACE OD NOWA
 *
 * NA EKRANIE MUSZA SIE POJAWIC INFORMACJE O STANIE POLACZNEIA I WYNIKACH DANYCH
 */