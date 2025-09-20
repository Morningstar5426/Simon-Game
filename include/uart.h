#include <stdint.h>
extern uint8_t sequence_length;
void uart_init(void);
uint8_t serial_bytes_available(void);
uint8_t uart_getc(void);
void uart_putc(uint8_t c);

extern volatile uint8_t uart_input;
extern volatile uint8_t reset_flag;
extern uint8_t current_step;
extern uint8_t current_sequence_length;
