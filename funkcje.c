/*
 * funkcje.c    atmega328p    F_CPU = 8000000 Hz
 *
 * Created on: 25.11.2017
 *     Author: admin
 */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include <util/atomic.h>
#include "komendy_at.h"
#include "uart.h"

//char buf[8];
volatile uint32_t timerCount = 0;
unsigned char read(uint8_t portAddress, uint8_t pinNumber) {
	unsigned char PRESENCE = 0;

	cbi2(portAddress, pinNumber);
	_delay_us(2);
	sbi2(portAddress, pinNumber);
	_delay_us(15);
	if (bit_is_set2(portAddress - 1, pinNumber)) {
		PRESENCE = 1;
	} else {
		PRESENCE = 0;
	}

	return (PRESENCE);
}
/*unsigned char RESET_PULSE(void) {
	unsigned char PRESENCE = -1;

	cbi2(4, 0);
	_delay_us(500);
	sbi2(4, 0);
	_delay_us(30);

	if (bit_is_clear2(4 - 1, 0)) {
		PRESENCE = 1;
	} else {
		PRESENCE = 0;
	}
	_delay_us(470);
	if (bit_is_set2(4 - 1, 0)) {
		PRESENCE = 1;
	} else {
		PRESENCE = 0;
	}

	return PRESENCE;

}*/

void send(unsigned char bit, uint8_t portAddress, uint8_t pinNumber) {

	cbi2(portAddress, pinNumber);
	_delay_us(5);
	if (bit == 1)
		sbi2(portAddress, pinNumber);
	_delay_us(80);
	sbi2(portAddress, pinNumber);

}

void send_byte(unsigned char wartosc, uint8_t portAddress, uint8_t pinNumber) {
	unsigned char i;
	unsigned char temp;

	for (i = 0; i < 8; i++) {
		temp = wartosc >> i;
		temp &= 0x01;
		send(temp, portAddress, pinNumber);
	}
	//dtostrf(wartosc, 1, 1, buf);
//	uart_puts(buf);
	_delay_us(100);
}

/*unsigned char read_byte(void) {
	unsigned char i;
	unsigned char wartosc = 0;

	for (i = 0; i < 8; i++) {
		//if (read())
		wartosc |= 0x01 << i;
		_delay_us(15);

	}
	return (wartosc);
}*/

/*int8_t startMeas(int count) {
	unsigned char sprawdz;
	int i;
	uint8_t licznik = 1;
	char temp1 = 0, temp2 = 0;
	uart_puts("POMIAR_TEMPERATURY");
	uart_puts(":\n");

	_delay_ms(200);

	for (i = 0; i < count; i++) {

		sprawdz = RESET_PULSE();
		// uart_putint(sprawdz,10);
		if (sprawdz == 1) {
			//	send_byte(0xCC);
			//send_byte(0x44);
			_delay_ms(750);

			sprawdz = RESET_PULSE();
			//send_byte(0xCC);
			//send_byte(0xBE);
			temp1 = read_byte();
			temp2 = read_byte();

			sprawdz = RESET_PULSE();

			float temp = 0;
			temp = (float) (temp1 + (temp2 * 256)) / 16;
			dtostrf(temp, 1, 1, buf);
			uart_puts("TEMP");
			uart_putint(licznik++, 10);
			uart_puts(":");
			uart_puts(buf);
			uart_puts("\n");
			_delay_ms(200);

		} else {
			uart_puts("CISZA");

		}

	}

	return 0;
}*/

void init_timer() {
	TCNT1 = timerCount;
	TCCR1A = 0x00;
	TCCR1B |= (1 << CS12) | (1 << CS10); // Timer mode with 1024 prescler
	TIMSK1 = (1 << TOIE1); // Enable timer1 overflow interrupt(TOIE1)    // set prescaler to 64 and starts PWM
}

unsigned int revBits(unsigned int data) {
	int pos = 8 - 1;

	int reverse = 0;

	while (pos >= 0 && data) {
		if (data & 1)
			reverse = reverse | (1 << pos);
		data >>= 1;
		pos--;
	}
	return reverse;
}
