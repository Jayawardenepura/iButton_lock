
#define F_CPU 16000000UL
/* bytes in read and write buffers */
/* !!! BUFFER_LEN is a max lenght of string, which is an argument of uart_put() */
#define BUFFER_LEN (65)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/atomic.h>

#include <stdbool.h>
#include <stdlib.h>

static const uint32_t uart_baudrate = 19200;	/* Baud rate (Baud / second) */

/* Value to be placed in UBRR register. See datasheet for more */
static const uint16_t ubrr_val = (F_CPU / (16 * uart_baudrate)) - 1;
/* Read and write buffers */
extern uint8_t rdbuff[];
extern uint8_t wrbuff[];
static uint8_t rdind = 0, wrind = 0;	/* Indices */
/* Indicates transfer & reception completion */
extern volatile bool txcflag;/* = true;*/
extern volatile bool rxcflag;




void uart_init(void);

void uart_put(char *str);

bool atomic_str_eq(char *str1, char *str2);

