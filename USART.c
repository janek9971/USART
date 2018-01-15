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

char bufor[100];    // bufor na potrzeby odebranych danych z UART

//************ g³ówna funkcja main() programu *****************************

void __init3(void) __attribute__ (( section( ".init3" ), naked, used ));
void __init3(void) {
	/* wy³¹czenie watchdoga (w tych mikrokontrolerach, w których watchdog
	 * ma mo¿liwoœæ generowania przerwania pozostaje on te¿ aktywny po
	 * resecie) */

	MCUSR = 0;
	WDTCSR = (1 << WDCE) | (1 << WDE);
	WDTCSR = 0;
}

int main(void) {
	eeprom_update_byte((uint8_t*) (addr), 0);
	eeprom_update_byte((uint8_t*) (addr + 1), 0);
	eeprom_update_byte((uint8_t*) (addr + 2), 0);
	eeprom_update_byte((uint8_t*) (addr + 3), 0);
	eeprom_update_byte((uint8_t*) (addr + 4), 0);
	eeprom_update_byte((uint8_t*) (addr + 5), 0);
	// inicjalizacja UART
	USART_Init( __UBRR);

	register_uart_str_rx_event_callback(parse_uart_data);

	sei();

	// przedstawienie siê uk³adu przez UART po starcie
	uart_puts("******* WITAJ ***********\r\n");

	while (1) {

		UART_RX_STR_EVENT(bufor);  // zdarzenie odbiorcze UART

	}

}

//************************* koniec main() **********************
