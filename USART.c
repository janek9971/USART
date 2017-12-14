/*
 * main.c
 *

 *
 */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

#include "uart.h"
#include "komendy_at.h"
#include <avr/eeprom.h>
char BUF[8];
char bufor[100];    // bufor na potrzeby odebranych danych z UART

int latch=0;
unsigned char sprawdz;
	int i;
	uint8_t licznik = 1;

//************ g³ówna funkcja main() programu *****************************

void __init3( void ) __attribute__ (( section( ".init3" ), naked, used ));
void __init3( void )
{
    /* wy³¹czenie watchdoga (w tych mikrokontrolerach, w których watchdog
     * ma mo¿liwoœæ generowania przerwania pozostaje on te¿ aktywny po
     * resecie) */

    MCUSR = 0;
    WDTCSR = (1<<WDCE) | (1<<WDE);
    WDTCSR = 0;
}

int main(void) {
	// DDRB = 255;
	// DDRC = 255;
	// DDRD = 255;

	/* latch=eeprom_read_byte((uint8_t*)5);
	 uart_putint(latch,5);
	 if(latch!=1){

		 eeprom_write_byte((uint8_t*) 5, 1);

	 }
	 else{
	 readValue = eeprom_read_byte((uint8_t*)addr);
	 DDRB=readValue;
	 PORTB=readValue;
	 readValue = 0;
	 readValue = eeprom_read_byte((uint8_t*)addr2);
	 DDRC=readValue;
	 PORTC=readValue;
	 	 readValue = 0;
	 readValue = eeprom_read_byte((uint8_t*)addr3);
	 DDRD=readValue;
	 PORTD=readValue;
	 readValue = 0;
	 }*/





	// inicjalizacja UART
	USART_Init( __UBRR);

	register_uart_str_rx_event_callback(parse_uart_data);

	sei();
	// globalne odblokowanie przerwañ


	// przedstawienie siê uk³adu przez UART po starcie
	uart_puts("******* WITAJ ***********\r\n");

	while (1) {

		UART_RX_STR_EVENT(bufor);  // zdarzenie odbiorcze UART

	}

}

//************************* koniec main() **********************
