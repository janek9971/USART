/*
 * uart.c
 *

 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/delay.h>
#include <util/atomic.h>

#include "uart.h"

volatile uint8_t ascii_line;


// definiujemy w ko�cu nasz bufor UART_RxBuf
volatile char UART_RxBuf[UART_RX_BUF_SIZE];
// definiujemy indeksy okre�laj�ce ilo�� danych w buforze
volatile uint8_t UART_RxHead; // indeks oznaczaj�cy �g�ow� w�a�
volatile uint8_t UART_RxTail; // indeks oznaczaj�cy �ogon w�a�



// definiujemy w ko�cu nasz bufor UART_RxBuf
volatile char UART_TxBuf[UART_TX_BUF_SIZE];
// definiujemy indeksy okre�laj�ce ilo�� danych w buforze
volatile uint8_t UART_TxHead; // indeks oznaczaj�cy �g�ow� w�a�
volatile uint8_t UART_TxTail; // indeks oznaczaj�cy �ogon w�a�


// wska�nik do funkcji callback dla zdarzenia UART_RX_STR_()
static void ( *uart_rx_str_event_callback )( char * pBuf );


// funkcja do rejestracji funkcji zwrotnej w zdarzeniu UART_RX_STR_EVENT()
void register_uart_str_rx_event_callback( void ( *callback )( char * pBuf ) ) {
    uart_rx_str_event_callback = callback;
}


// Zdarzenie do odbioru danych �a�cucha tekstowego z bufora cyklicznego
void UART_RX_STR_EVENT( char * rbuf ) {

    if ( ascii_line ) {
        if ( uart_rx_str_event_callback ) {
            uart_get_str( rbuf );
            ( *uart_rx_str_event_callback )( rbuf );
        } else {
            UART_RxHead = UART_RxTail;
        }
    }
}



void USART_Init( uint16_t ubrr ) {
    /*Set baud rate */
    UBRR0H = ( uint8_t )( ubrr >> 8 );
    UBRR0L = ( uint8_t )ubrr;
    UCSR0B = ( 1 << RXEN0 ) | ( 1 << TXEN0 ) | ( 1 << RXCIE0 );
    /* Set frame format: 8data, 1stop bit */
    UCSR0C = ( 3 << UCSZ00 );
}


#ifdef UART_DE_PORT
ISR( USART_TXC_vect ) {
    UART_DE_ODBIERANIE;    // zablokuj nadajnik RS485
}
#endif

// definiujemy funkcj� dodaj�c� jeden bajtdoz bufora cyklicznego
void uart_putc( char data ) {
    uint8_t tmp_head;
    ATOMIC_BLOCK( ATOMIC_RESTORESTATE ) {
        tmp_head  = ( UART_TxHead + 1 ) & UART_TX_BUF_MASK;
    }
    // p�tla oczekuje je�eli brak miejsca w buforze cyklicznym na kolejne znaki
    while ( tmp_head == UART_TxTail ) {}

    UART_TxBuf[tmp_head] = data;
    UART_TxHead = tmp_head;

    // inicjalizujemy przerwanie wyst�puj�ce, gdy bufor jest pusty, dzi�ki
    // czemu w dalszej cz�ci wysy�aniem danych zajmie si� ju� procedura
    // obs�ugi przerwania
    UCSR0B |= ( 1 << UDRIE0 );
}


void uart_puts( char *s ) {    // wysy�a �a�cuch z pami�ci RAM na UART
    register char c;
    while (( c = *s++ ) ) uart_putc( c );       // dop�ki nie napotkasz 0 wysy�aj znak
}

void uart_putint( int value, int radix ) { // wysy�a na port szeregowy tekst
    char string[17];            // bufor na wynik funkcji itoa
    itoa( value, string, radix );      // konwersja value na ASCII
    uart_puts( string );          // wy�lij string na port szeregowy
}


// definiujemy procedur� obs�ugi przerwania nadawczego, pobieraj�c� dane z bufora cyklicznego
ISR( USART_UDRE_vect )  {
    // sprawdzamy czy indeksy s� r�ne
    if ( UART_TxHead != UART_TxTail ) {
        // obliczamy i zapami�tujemy nowy indeks ogona w�a (mo�e si� zr�wna� z g�ow�)
        UART_TxTail = ( UART_TxTail + 1 ) & UART_TX_BUF_MASK;
        // zwracamy bajt pobrany z bufora  jako rezultat funkcji
#ifdef UART_DE_PORT
        UART_DE_NADAWANIE;
#endif
        UDR0 = UART_TxBuf[UART_TxTail];
    } else {
        // zerujemy flag� przerwania wyst�puj�cego gdy bufor pusty
        UCSR0B &= ~( 1 << UDRIE0 );
    }
}


// definiujemy funkcj� pobieraj�c� jeden bajt z bufora cyklicznego
int uart_getc( void ) {
    int data = -1;
    // sprawdzamy czy indeksy s� r�wne
    if ( UART_RxHead == UART_RxTail ) return data;
    ATOMIC_BLOCK( ATOMIC_RESTORESTATE ) {
        // obliczamy i zapami�tujemy nowy indeks �ogona w�a� (mo�e si� zr�wna� z g�ow�)
        UART_RxTail = ( UART_RxTail + 1 ) & UART_RX_BUF_MASK;
        // zwracamy bajt pobrany z bufora  jako rezultat funkcji
        data = UART_RxBuf[UART_RxTail];
    }
    return data;
}

char * uart_get_str( char * buf ) {
    int c;
    char * wsk = buf;
    if ( ascii_line ) {
        while (( c = uart_getc() ) ) {
            if ( 13 == c || c < 0 ) break;
            *buf++ = c;
        }
        *buf = 0;
        ascii_line--;
    }
    return wsk;
}



// definiujemy procedur� obs�ugi przerwania odbiorczego, zapisuj�c� dane do bufora cyklicznego
ISR( USART_RX_vect ) {

    register uint8_t tmp_head;
    register char data;

    data = UDR0; //pobieramy natychmiast bajt danych z bufora sprz�towego

    // obliczamy nowy indeks �g�owy w�a�
    tmp_head = ( UART_RxHead + 1 ) & UART_RX_BUF_MASK;

    // sprawdzamy, czy w�� nie zacznie zjada� w�asnego ogona
    if ( tmp_head == UART_RxTail ) {

        UART_RxHead = UART_RxTail;
    } else {
        switch ( data ) {
        case 0:                    // ignorujemy bajt = 0
        case 10:
            break;            // ignorujemy znak LF
        case 13:
            ascii_line++;    // sygnalizujemy obecno�� kolejnej linii w buforze
        default :
            UART_RxHead = tmp_head;
            UART_RxBuf[tmp_head] = data;
        }

    }
}
