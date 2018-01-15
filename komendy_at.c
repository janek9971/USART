/*
 * komendy_at.c
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
#include "funkcje.h"
#include "komendy_at.h"
#include <stdbool.h>
#include <avr/eeprom.h>

char * paramsPointer;
int addr = 1, addr2 = 2, addr3 = 3, tempCount = 0, readbuf[9], globalStop = 0,
		period = 0, readValue = 0;
char sensorValue;
bool state;
char temp1 = 0, temp2 = 0;
int readtime = 1;
bool correct = true;

#define CMD_CNT   10 // iloæ poleceñ
//----------- tablica z poleceniami i wskaŸnikami funkcji do ich obs³ugi --------------------
const CMD comm[CMD_CNT] PROGMEM = { { "TEST", testing_comm }, { "SENDBYTE",
		comm_to_send_byte }, { "READBYTE", comm_to_read_byte }, { "RST",
		rst_service }, { "PORTS", ports_control }, { "SAVEM", save_eeprom }, {
		"RST+MEM", rst_service_cleareeprom }, { "READM", read_eeprom }, { "PIN",
		port_control }, { "WAIT", comm_to_wait }, };

//----------------- funkcja do analizowania danych odebranych z UART ------------------------------
void parse_uart_data(char * pBuf) {

	int8_t (*_at_srv)(uint8_t inout, char * data);

	char * cmd_wsk;

	char * rest;
	uint8_t i = 0, len;

	if (strpbrk(pBuf, "=?")) {
		// obs³uga poleceñ AT we/wy + parametry

		if (strpbrk(pBuf, "?")) {
			// zapytania do uk³adu w postaci: AT+CMD?

			cmd_wsk = strtok_r(pBuf, "?", &rest);
			len = strlen(cmd_wsk);
			for (i = 0; i < CMD_CNT; i++) {
				if (len && 0 == strncasecmp_P(cmd_wsk, comm[i].command, len)) {
					if (pgm_read_word(comm[i].command)) {
						_at_srv = (void *) pgm_read_word(&comm[i].comm_service);
						if (_at_srv) {
							if (_at_srv(0, rest) < 0)
								uart_puts("UNKNOWN COMMAND\r\n");
						}
					}
					uart_puts("SUCCESS!\r\n");
					break;
				}
			}

		} else {
			// ustawienia uk³adu w postaci: AT+CMD=parametry

			cmd_wsk = strtok_r(pBuf, "=", &rest);

			len = strlen(cmd_wsk);
			for (i = 0; i < CMD_CNT; i++) {
				if (len && 0 == strncasecmp_P(cmd_wsk, comm[i].command, len)) {
					if (pgm_read_word(comm[i].command)) {
						_at_srv = (void *) pgm_read_word(&comm[i].comm_service);
						if (_at_srv && !_at_srv(1, rest)) {
							if (strcmp(cmd_wsk, "READBYTE") == 0) {

							} else if (strcmp(cmd_wsk, "SENDBYTE")
									== 0&& correct!=false) {
								cmd_wsk = strtok_r(0, ",", &rest);
								uart_puts("SENDING: ");
								uart_puts(cmd_wsk);
								uart_puts("\r\n");

							}

							else {
								if (correct == true)
									uart_puts("SUCCESS!\r\n");
								correct = true;
							}
						} else {
							uart_puts("UNKNOWN COMMAND\r\n");
						}
					}
					break;
				}
			}
		}

	}

	if ( CMD_CNT == i)
		uart_puts("UNKNOWN COMMAND\r\n");
}

//----------------- obs³uga poszczególnych komend ----------------------------------
int8_t testing_comm(uint8_t inout, char * params) {

	uart_puts("OK!\r\n");
	correct = true;
	return 0;
}
int8_t comm_to_send_byte(uint8_t inout, char * params) {
	uint8_t portAddress = 0, pinNumber;

	paramsPointer = strtok(params, ",");
	char * dane = paramsPointer;
	unsigned char parsedData = 0;
	paramsPointer = strtok(0, ",");

	portAddress = atoi(paramsPointer);
	if (strcmp(paramsPointer, "DDRB") == 0) {
		portAddress = 4;
	} else if (strcmp(paramsPointer, "DDRC") == 0) {
		portAddress = 7;
	} else if (strcmp(paramsPointer, "DDRD") == 0) {
		portAddress = 10;

	} else {
		correct = false;

	}
	paramsPointer = strtok(0, ",");
	if (strcmp(paramsPointer, "") == 0) {
		correct = false;
	}
	pinNumber = atoi(paramsPointer);
	if (pinNumber < 0 || pinNumber > 7) {
		correct = false;
	}
	if (strchr(dane, 'b') != NULL) {
		parsedData = (unsigned char) strtoul(dane, NULL, 2);
	} else if (strchr(dane, 'x') != NULL || strchr(dane, 'X') != NULL) {
		parsedData = (unsigned char) strtoul(dane, NULL, 16);
	} else if (atoi(dane) != 0) {
		parsedData = (unsigned char) strtoul(dane, NULL, 10);
	} else {
		correct = false;

	}
	if (correct != false) {

		send_byte((unsigned char) parsedData, portAddress, pinNumber);
	} else {
		uart_puts("INVALID PARAMETERS\r\n");
	}

	return 0;
}

int8_t comm_to_read_byte(uint8_t inout, char * params) {

	uint8_t portAddress = 0, pinNumber;
	if (tempCount == 10) {
		tempCount = 0;
	}

	unsigned char i;
	unsigned char value = 0;
	paramsPointer = strtok(params, ",");

	portAddress = atoi(paramsPointer);
	if (strcmp(paramsPointer, "DDRB") == 0) {
		portAddress = 4;
	} else if (strcmp(paramsPointer, "DDRC") == 0) {
		portAddress = 7;
	} else if (strcmp(paramsPointer, "DDRD") == 0) {
		portAddress = 10;
	} else {
		correct = false;

	}

	paramsPointer = strtok(0, ",");
	if (strcmp(paramsPointer, "") == 0) {
		correct = false;
	}
	pinNumber = atoi(paramsPointer);
	if (pinNumber < 0 || pinNumber > 7) {
		correct = false;
	}
	if (correct != false) {
		for (i = 0; i < 8; i++) {
			if (read(portAddress, pinNumber))
				value |= 0x01 << i;
			_delay_us(15);

		}
		readbuf[tempCount] = value;
		uart_puts("READED_VALUE: ");
		uart_putint(readbuf[tempCount], 10);
		uart_puts("\r\n");
		tempCount++;
	} else {
		uart_puts("INVALID PARAMETERS\r\n");
	}

	return 0;
}

int8_t rst_service(uint8_t inout, char * params) {

	uart_puts("za 0.2s - restart\r\n");

	_delay_ms(200);

	cli();
	// wy³¹cz przerwania
	wdt_enable(0);// ustaw watch-dog
	while (1)
		;
	// czekaj na RESET

	return 0;
}

int8_t rst_service_cleareeprom(uint8_t inout, char * params) {

	uart_puts("za 0.2s - restart\r\n");

	_delay_ms(200);

	cli();
	// wy³¹cz przerwania

	eeprom_update_byte((uint8_t*) (addr), 0);
	eeprom_update_byte((uint8_t*) (addr + 1), 0);
	eeprom_update_byte((uint8_t*) (addr + 2), 0);
	eeprom_update_byte((uint8_t*) (addr + 3), 0);
	eeprom_update_byte((uint8_t*) (addr + 4), 0);
	eeprom_update_byte((uint8_t*) (addr + 5), 0);
	wdt_enable(0);  // ustaw watch-dog
	while (1)
		;
	// czekaj na RESET

	return 0;
}

int8_t ports_control(uint8_t inout, char * params) {

	char * asciiPortB, *asciiPortC, *asciiPortD;

	uint8_t binaryPortB = 0, binaryPortC = 0, binaryPortD = 0, option;

	paramsPointer = strtok(params, ",");
	option = atoi(paramsPointer);
	paramsPointer = strtok(0, ",");
	asciiPortB = paramsPointer;
	paramsPointer = strtok(0, ",");
	asciiPortC = paramsPointer;
	paramsPointer = strtok(0, ",");
	asciiPortD = paramsPointer;
	paramsPointer = strtok(0, ",");
	period = atoi(paramsPointer);
	timerCount = 65536 - (8000000 / 1024 / (1000 / period));
	binaryPortB = strtoul(asciiPortB, NULL, 2);
	binaryPortC = strtoul(asciiPortC, NULL, 2);
	binaryPortD = strtoul(asciiPortD, NULL, 2);
	binaryPortB = revBits(binaryPortB);
	binaryPortC = revBits(binaryPortC);
	binaryPortD = revBits(binaryPortD);
	if (period != 0) {
		init_timer();
	}
	if (option == 0) {

		PORTB = binaryPortB;
		PORTC = binaryPortC;
		PORTD = binaryPortD;
	} else if (option == 1) {
		DDRB = binaryPortB;
		DDRC = binaryPortC;
		DDRD = binaryPortD;
	} else if (option == 2 && period != 0) {
		while (1) {

			if (state == false) {

				PORTB ^= binaryPortB;
				PORTC ^= binaryPortC;
				PORTD ^= binaryPortD;
				state = true;
			}
			if (globalStop == 1) {
				globalStop = 0;
				break;

			}

		}
	} else {
		correct = false;

	}
	if(correct==false){
		uart_puts("INVALID PARAMETERS\r\n");
	}
	return 0;

}

int8_t port_control(uint8_t inout, char * params) {

	uint8_t pinstate;
	uint8_t portAddress = 0, pinNumber;
	paramsPointer = strtok(params, ",");

	if (strcmp(paramsPointer, "DB") == 0) {
		portAddress = 4;

	} else if (strcmp(paramsPointer, "DC") == 0) {
		portAddress = 7;
	} else if (strcmp(paramsPointer, "DD") == 0) {
		portAddress = 10;

	} else if (strcmp(paramsPointer, "PB") == 0) {
		portAddress = 5;

	} else if (strcmp(paramsPointer, "PC") == 0) {

		portAddress = 8;
	} else if (strcmp(paramsPointer, "PD") == 0) {

		portAddress = 11;
	} else {
		correct = false;
	}

	paramsPointer = strtok(0, ",");
	if (strcmp(paramsPointer, "") == 0) {
		correct = false;
	}
	pinNumber = atoi(paramsPointer);
	paramsPointer = strtok(0, ",");
	if (strcmp(paramsPointer, "") == 0) {
		correct = false;
	}
	pinstate = atoi(paramsPointer);
	if (pinNumber < 0 || pinNumber > 7) {
		correct = false;
	}

	if (pinstate == 1) {
		setbit(portAddress, pinNumber);

	} else if (pinstate == 0) {
		clearbit(portAddress, pinNumber);

	} else {
		correct = false;

	}
	if (correct == false) {
		uart_puts("INVALID PARAMETERS\r\n");
	}
	return 0;

}

int8_t save_eeprom(uint8_t inout, char * params) {

	sensorValue = DDRB;
	eeprom_write_byte((uint8_t*) (addr), (char) sensorValue);
	sensorValue = 0;
	sensorValue = DDRC;
	eeprom_write_byte((uint8_t*) (addr + 1), (uint8_t) sensorValue);
	sensorValue = 0;
	sensorValue = DDRD;
	eeprom_write_byte((uint8_t*) (addr + 2), (uint8_t) sensorValue);
	sensorValue = 0;
	sensorValue = PORTB;
	eeprom_write_byte((uint8_t*) (addr + 3), (uint8_t) sensorValue);
	sensorValue = 0;
	sensorValue = PORTC;
	eeprom_write_byte((uint8_t*) (addr + 4), (uint8_t) sensorValue);
	sensorValue = 0;
	sensorValue = PORTD;
	eeprom_write_byte((uint8_t*) (addr + 5), (uint8_t) sensorValue);
	sensorValue = 0;

	return 0;
}

int8_t read_eeprom(uint8_t inout, char * params) {
	int j = 0;

	readValue = (char) eeprom_read_byte((uint8_t*) addr);
	if (readValue == 0) {
		j++;
	} else {
		DDRB = readValue;
	}
	readValue = 0;
	readValue = (char) eeprom_read_byte((uint8_t*) (addr + 1));
	if (readValue == 0) {
		j++;
	} else {
		DDRC = readValue;
	}
	readValue = 0;
	readValue = (char) eeprom_read_byte((uint8_t*) (addr + 2));
	if (readValue == 0) {
		j++;
	} else {
		DDRD = readValue;
	}
	readValue = 0;
	readValue = (char) eeprom_read_byte((uint8_t*) (addr + 3));
	if (readValue == 0) {
		j++;
	} else {
		PORTB = readValue;
	}
	readValue = 0;
	readValue = (char) eeprom_read_byte((uint8_t*) (addr + 4));
	if (readValue == 0) {
		j++;
	} else {
		PORTC = readValue;
	}
	readValue = 0;
	readValue = (char) eeprom_read_byte((uint8_t*) (addr + 5));
	if (readValue == 0) {
		j++;
	} else {
		PORTD = readValue;
	}
	if (j == 6) {
		uart_puts("NO SAVED DATA");
	}
	readValue = 0;

	return 0;
}

int8_t comm_to_wait(uint8_t inout, char * params) {

	uint8_t option;
	uint32_t timeToWait;
	paramsPointer = strtok(params, ",");
	option = atoi(paramsPointer);
	paramsPointer = strtok(0, ",");
	timeToWait = atoi(paramsPointer);
	if (option == 0) {

		while (timeToWait--) {
			_delay_ms(1);  // one millisecond
		}

	} else if (option == 1) {
		while (timeToWait--) {
			_delay_us(1);  // one microsecond
		}
	}

	else {
		correct = false;
		uart_puts("INVALID\r\n");

	}

	return 0;
}

ISR (TIMER1_OVF_vect)    // Timer1 ISR
{

	state = false;
	TCNT1 = timerCount;   // for 1 sec at 16 MHz

}

/*
 * Zatrzymanie TIMERA
 TCCR1B &= ~(1 << CS10);
 TCCR1B &= ~(1 << CS12);
 */
