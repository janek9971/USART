/*
 * komendy_at.h
 *

 */

#ifndef KOMENDY_AT_H_
#define KOMENDY_AT_H_

// definicje na potrzeby programu
#define LED         (1<<PB0)
#define LED_DIR     DDRB
#define LED_PORT     PORTB
#define LED_PIN     PINB

#define LED_OFF     LED_PORT &= ~LED
#define LED_ON         LED_PORT |= LED
#define LED_TOG     LED_PORT ^= LED

// definicja typu strukturalnego
typedef struct {
	char polecenie_at[8];
	int8_t (*at_service)(uint8_t inout, char * params);
}const TATCMD;

// deklaracje zmiennych zewnêtrznych
extern TATCMD polecenia_at[] PROGMEM;
//extern uint8_t portAddress;
//extern uint8_t pinNumber;


// deklaracje funkcji
void parse_uart_data(char * pBuf);

int8_t comm_to_send_byte(uint8_t inout, char * params);
int8_t comm_to_read_byte(uint8_t inout, char * params);
int8_t ports_control(uint8_t inout, char * params);
int8_t rst_service(uint8_t inout, char * params);
//int8_t reset_pulse(uint8_t inout, char * params);
int8_t measurment(uint8_t inout, char * params);
int8_t test(uint8_t inout, char * params);
int8_t stop(uint8_t inout, char * params);
int8_t save_eeprom(uint8_t inout, char * params);
int8_t rst_service_cleareeprom(uint8_t inout, char * params);
int8_t read_eeprom(uint8_t inout, char * params);
int8_t port_control(uint8_t inout, char * params);
int8_t comm_to_wait(uint8_t inout, char * params);
void delay_ms(int ms);
#endif /* KOMENDY_AT_H_ */
