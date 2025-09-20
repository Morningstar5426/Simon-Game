#include "timers.h"

#include <stdint.h>
#include "display.h"
#include <avr/io.h>
#include <avr/interrupt.h>


volatile uint16_t elapsed_time = 0;

void spi_write(uint8_t data) {
    SPI0.DATA = data; // Note DATA register used for both Tx and Rx
}

// Interrupt for writing to spi and handling pushbuttons
ISR(TCB1_INT_vect) {
    static uint8_t current_side = 0;
    static uint8_t count0 = 0;
    static uint8_t count1 = 0;

    // 1. Display multiplexing
    if (current_side) {
        spi_write(left_segment | (0x01 << 7));
    } else {
        spi_write(right_segment);
    }
    current_side = !current_side;

    // 2. Pushbutton debouncing
    uint8_t pb_edge = pb_debounced_state ^ PORTA.IN;

    // Vertical counter for debouncing
    count1 = (count1 ^ count0) & pb_edge;
    count0 = ~count0 & pb_edge;

    // Update pb_debounced_state upon consistent samples
    pb_debounced_state ^= (count1 & count0);

    // Acknowledge interrupt
    TCB1.INTFLAGS = TCB_CAPT_bm;
}

// Interrupt for measuring time 
ISR(TCB0_INT_vect)
{
    elapsed_time++;
    TCB0.INTFLAGS = TCB_CAPT_bm;
}

uint16_t playback_delay = 250;


void update_playback_delay(void)
{
    ADC0.COMMAND = ADC_START_IMMEDIATE_gc;
    while (!(ADC0.INTFLAGS & ADC_RESRDY_bm));
    uint16_t adc_result = ADC0.RESULT;
    playback_delay = 250 + (((adc_result + 1) * 1750) >> 8);
    ADC0.INTFLAGS = ADC_RESRDY_bm;

}