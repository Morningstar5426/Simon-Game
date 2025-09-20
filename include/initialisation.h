#include <stdint.h>

// Initialises the petentiometer
void adc_init(void);

/**
 * Configures the sending and receiving of data through uart.
 * Sets up the standard output stream
 */
void uart_init(void);

/**
 * Configures PWM through TCA0.
 *
 * @note - PER is set to 1 and CMP0 is set to 0.
 * @warning - PB0 must be output enabled.
 */
void pwm_init(void);

/**
 * Enabls pull-up resistors for PBs (active low buttons)
 */
void buttons_init(void);

/**
 * Configures TCB0 for timing events and configures TCB1 for pushbutton
 * debouncing and display multiplexing.
 *
 * @note - TCB0 generates interrupts every 1 ms.
 * @note - TCB1 generates interrupts every 10 ms.
 * @warning - Interrupts must be enabled globally.
 */
void timer_init(void);

// Counts the elapsed time in ms
extern volatile uint16_t elapsed_time;

// A debounced sample of the pushbuttons
extern volatile uint8_t pb_debounced_state;
