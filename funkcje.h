/*
 * funkcje.h    atmega328p    F_CPU = 8000000 Hz
 *
 * Created on: 25.11.2017
 *     Author: admin
*/

#ifndef FUNKCJE_H_
#define FUNKCJE_H_
#define DIR     DDRB
#define PORT    PORTB
#define PIN     PINB

 unsigned char RESET_PULSE( void );
int8_t startMeas(int count);
unsigned char read_byte(void) ;
void send_byte(char wartosc, uint8_t portAddress, uint8_t pinNumber)  ;
unsigned char read(uint8_t portAddress, uint8_t pinNumber) ;
void send(char bit, uint8_t portAddress, uint8_t pinNumber );
extern void init_timer();
extern volatile uint32_t timerCount;
extern unsigned int revBits(unsigned int data);

#endif  /* FUNKCJE_H_ */
