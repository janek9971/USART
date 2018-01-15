/*
 * uart.h
 *

 */

#ifndef UART_H_
#define UART_H_
//parametry transmisji
#define UART_BAUD 9600
#define __UBRR ((F_CPU+UART_BAUD*8UL) / (16UL*UART_BAUD)-1)

#define UART_RX_BUF_SIZE 128 // definicja buforu o rozmiarze 128 bajtów
// definicja maski buforu
#define UART_RX_BUF_MASK ( UART_RX_BUF_SIZE - 1)

#define UART_TX_BUF_SIZE 32 // definicja buforu o rozmiarze 32 bajtów
// definicja maski buforu
#define UART_TX_BUF_MASK ( UART_TX_BUF_SIZE - 1)
#define cbi(sfr,bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define bit_is_clear2(sfr, bit) (!(_SFR_IO8(sfr) & _BV(bit)))
#define bit_is_set2(sfr, bit) (_SFR_IO8(sfr) & _BV(bit))
#define clearbit(sfr,bit) (_SFR_IO8(sfr) &=~_BV(bit))
#define setbit(sfr,bit) (_SFR_IO8(sfr) |= _BV(bit))

//funckja zewnêtrzna
extern volatile uint8_t ascii_line;

// deklaracje funkcji
void USART_Init(uint16_t ubrr);
int uart_getc(void);
void uart_putc(char data);
void uart_puts(char *s);
void uart_putint(int value, int base);
char * uart_get_str(char * buf);
void UART_RX_STR_EVENT(char * rbuf);
void register_uart_str_rx_event_callback(void (*callback)(char * pBuf));

#endif /* UART_H_ */








