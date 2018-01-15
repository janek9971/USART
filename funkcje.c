/*
 * funkcje.c
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

volatile uint32_t timerCount = 0;
unsigned char read(uint8_t portAddress, uint8_t pinNumber) {
	unsigned char PRESENCE = 0;
	setbit(portAddress, pinNumber);
	_delay_us(2);
	clearbit(portAddress, pinNumber);
	_delay_us(15);
	if (bit_is_set2(portAddress - 1, pinNumber)) {
			PRESENCE = 1;
		} else {
			PRESENCE = 0;
		}

		return (PRESENCE);
}

void send(unsigned char bit, uint8_t portAddress, uint8_t pinNumber) {

	setbit(portAddress, pinNumber);
	_delay_us(5);
	if (bit == 1)
		clearbit(portAddress, pinNumber);
	_delay_us(80);
	clearbit(portAddress, pinNumber);

}

void send_byte(unsigned char wartosc, uint8_t portAddress, uint8_t pinNumber) {
	unsigned char i;
	unsigned char temp;

	for (i = 0; i < 8; i++) {
		temp = wartosc >> i;
		temp &= 0x01;
		send(temp, portAddress, pinNumber);
	}

	_delay_us(100);
}

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
