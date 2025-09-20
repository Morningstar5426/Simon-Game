#include <stdio.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "initialisation.h"


void adc_init(void)
{
    ADC0.CTRLA = ADC_ENABLE_bm;                              // Enable ADC0
    ADC0.CTRLB = ADC_PRESC_DIV2_gc;                          // Set prescaler to 2
    ADC0.CTRLC = (4 << ADC_TIMEBASE_gp) | ADC_REFSEL_VDD_gc; // Set reference voltage to VDD
    ADC0.CTRLE = 64;                                         // Sample duration
    ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc;                        // Select ADC channel AIN2 (PA2)
}


void pwm_init(void)
{
    PORTB.DIRSET = PIN0_bm;
    // Enable output override on PB0.
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm;

    // PWM initially OFF
    TCA0.SINGLE.PER = 1;
    TCA0.SINGLE.CMP0 = 0;
    TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm | TCA_SINGLE_CLKSEL_DIV2_gc;

    // Enable TCA0
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
}

// Handles sending of uart input
int uart_putc(char c, FILE *stream)
{
    while (!(USART0.STATUS & USART_DREIF_bm))
        ; // Wait for TXDATA empty
    USART0.TXDATAL = c;

    return c;
}
static FILE stdio = FDEV_SETUP_STREAM(uart_putc, NULL, _FDEV_SETUP_WRITE);


void uart_init(void)
{
    // 9600 baud
    USART0.BAUD = 1389;
    PORTB.DIRSET = PIN2_bm;

    // Enable receive complete interrupt
    USART0.CTRLA = USART_RXCIE_bm;
    USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;


    stdout = &stdio;
}


void buttons_init(void)
{

    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;

}



volatile uint8_t pb_debounced_state = 0xFF;

void timer_init(void)
{
    // 1ms interrupt for elapsed time
    TCB0.CCMP = 3333;
    TCB0.INTCTRL = TCB_CAPT_bm;
    TCB0.CTRLA = TCB_ENABLE_bm;
    
    TCB1.CCMP = 16667;               // Set interval for 5 ms (16667 clocks @ 3.333 MHz)
    TCB1.INTCTRL = TCB_CAPT_bm;      // CAPT interrupt enable
    TCB1.CTRLA = TCB_ENABLE_bm;      // Enable

}



