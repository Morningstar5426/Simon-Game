#include "display.h"
#include "display_macros.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "initialisation.h"

    //    ABCDEFG    xFABGCDE
    // 0: 0000001    00001000    0x08
    // 1: 1001111    01101011    0x6B
    // 2: 0010010    01000100    0x44
    // 3: 0000110    01000001    0x41
    // 4: 1001100    00100011    0x23
    // 5: 0100100    00010001    0x11
    // 6: 0100000    00010000    0x10
    // 7: 0001111    01001011    0x4B
    // 8: 0000000    00000000    0x00
    // 9: 0000100    00000001    0x01
volatile uint8_t segs [] = {
    0x08,0x6B,0x44,0x41,0x23,0x11,0x10,0x4B,0x00,0x01
};


void find_digits (uint8_t num, uint8_t* d1, uint8_t* d2) {
    if (num < 10) {
        *d1 = 0xFF; 
        *d2 = num;
    } else {
        *d1 = num / 10;
        *d2 = num % 10;
    }
    if (*d1 == 0xFF) {
        left_segment = DISP_OFF; // Sets the lhs to off if < 10
    } else {
        left_segment = segs[*d1];
    }
    
    right_segment = segs[*d2]; 
}



uint8_t left_segment;
uint8_t right_segment;

//Turns the display off
void display_off(void){
    left_segment = DISP_OFF;
    right_segment = DISP_OFF;
}

// Initialises the ability to write to certain parts of the display 
void spi_init(void){
    PORTMUX.SPIROUTEA = PORTMUX_SPI0_ALT1_gc;  // SPI pins on PC0-3

    PORTC.DIRSET = (PIN0_bm | PIN2_bm);    // SCK (PC0) and MOSI (PC2) output

    PORTA.OUTSET = PIN1_bm;     // DISP_LATCH initial high
    PORTA.DIRSET = PIN1_bm;     // set PORTA pin 1 to output

    SPI0.CTRLA = SPI_MASTER_bm;    // Master, /4 prescaler, MSB first
    SPI0.CTRLB = SPI_SSD_bm;       // Mode 0, client select disable, unbuffered
    SPI0.INTCTRL = SPI_IE_bm;      // Interrupt enable
    SPI0.CTRLA |= SPI_ENABLE_bm;   // Enable
}


// Interrupt for the display
ISR(SPI0_INT_vect){
    //rising edge on DISP_LATCH
    PORTA.OUTCLR = PIN1_bm;
    PORTA.OUTSET = PIN1_bm;  
    SPI0.INTFLAGS = SPI_IF_bm;
}




void update_display(const uint8_t left, const uint8_t right) {
    left_segment = left | PIN7_bm;
    right_segment = right;
}
