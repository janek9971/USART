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

volatile char UART_RxBuf[UART_RX_BUF_SIZE];

volatile uint8_t UART_RxHead;
volatile uint8_t UART_RxTail;

volatile char UART_TxBuf[UART_TX_BUF_SIZE];
volatile uint8_t UART_TxHead;
volatile uint8_t UART_TxTail;

// zdarzenie UART_RX_STR_()
static void (*uart_rx_str_event_callback)(char * pBuf);

//rejestracja funkcji zwrotnej w zdarzeniu UART_RX_STR_EVENT()
void register_uart_str_rx_event_callback(void (*callback)(char * pBuf)) {
	uart_rx_str_event_callback = callback;
}

// Zdarzenie do odbioru danych ³añcucha tekstowego z bufora cyklicznego
void UART_RX_STR_EVENT(char * rbuf) {

	if (ascii_line) {
		if (uart_rx_str_event_callback) {
			uart_get_str(rbuf);
			(*uart_rx_str_event_callback)(rbuf);
		} else {
			UART_RxHead = UART_RxTail;
		}
	}
}

void USART_Init(uint16_t ubrr) {
	/*Set baud rate */
	UBRR0H = (uint8_t) (ubrr >> 8);
	UBRR0L = (uint8_t) ubrr;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	/* Set frame format: 8data, 1stop bit */
	UCSR0C = (3 << UCSZ00);
}

// definiujemy funkcjê dodaj¹c¹ jeden bajt doz bufora cyklicznego
void uart_putc(char data) {
	uint8_t tmp_head;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		tmp_head = (UART_TxHead + 1) & UART_TX_BUF_MASK;
	}
	while (tmp_head == UART_TxTail) {
	}

	UART_TxBuf[tmp_head] = data;
	UART_TxHead = tmp_head;
	UCSR0B |= (1 << UDRIE0);
}

void uart_puts(char *s) {    // funkcja wysy³aj¹ca ³añcuch z pamiêci RAM na UART
	register char c;
	while ((c = *s++))
		uart_putc(c);
}

void uart_putint(int value, int base) { // wysy³a na port szeregowy tekst
	char string[17];            // bufor na wynik funkcji itoa
	itoa(value, string, base);      // konwersja value na ASCII
	uart_puts(string);          // wys³anie ci¹gu znaków na port szeregowy
}

// procedura obs³ugi przerwania nadawczego, pobieraj¹ca dane z bufora cyklicznego
ISR( USART_UDRE_vect) {
	// sprawdzamy czy indeksy s¹ ró¿ne
	if (UART_TxHead != UART_TxTail) {

		UART_TxTail = (UART_TxTail + 1) & UART_TX_BUF_MASK;

		UDR0 = UART_TxBuf[UART_TxTail];
	} else {
		// zerowanie flagi przerwania wystêpuj¹cej gdy bufor pusty
		UCSR0B &= ~(1 << UDRIE0);
	}
}

// funkcja pobieraj¹c¹ jeden bajt z bufora cyklicznego
int uart_getc(void) {
	int data = -1;
	// sprawdzamy czy indeksy s¹ równe
	if (UART_RxHead == UART_RxTail)
		return data;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		// obliczanie i przypisywanie nowego indeksu „ogona wê¿a”
		UART_RxTail = (UART_RxTail + 1) & UART_RX_BUF_MASK;
		// zwracany jest bajt pobrany z bufora  jako rezultat funkcji
		data = UART_RxBuf[UART_RxTail];
	}
	return data;
}

char * uart_get_str(char * buf) {
	int c;
	char * wsk = buf;
	if (ascii_line) {
		while ((c = uart_getc())) {
			if (13 == c || c < 0)
				break;
			*buf++ = c;
		}
		*buf = 0;
		ascii_line--;
	}
	return wsk;
}

// definicja procedury obs³ugi przerwania odbiorczego, zapisuj¹ca dane do bufora cyklicznego
ISR( USART_RX_vect) {

	register uint8_t tmp_head;
	register char data;

	data = UDR0; //pobranie bezpoœrednio bajtu danych z bufora sprzêtowego

	// obliczanie nowego indeksu „g³owy wê¿a”
	tmp_head = (UART_RxHead + 1) & UART_RX_BUF_MASK;

	// sprawdzenie czy w¹¿ nie "zjada" w³asnego ogona
	if (tmp_head == UART_RxTail) {

		UART_RxHead = UART_RxTail;
	} else {
		switch (data) {
		case 0:                    // ignorujemy bajt = 0
		case 10:
			break;            // ignorujemy znak LF (koniec linii)
		case 13:			 // zliczamy znak CR (rozpoczêcie nowej linii)
			ascii_line++;    // sygnalizacja nowej kolejnej linii w buforze

		default:
			UART_RxHead = tmp_head;
			UART_RxBuf[tmp_head] = data;
		}

	}
}
