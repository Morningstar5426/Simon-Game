#include <stdint.h>

/**
 * Turns the display off by writing 0b01111111 to both sides.
 */
void display_off(void);


/**
 * Update the bytes transmitted to the 7-segment display.
 *
 * @param left The left display value.
 * @param right The right display value.
 *
 * @note - Ensure bit 7 is cleared for both parameters.
 */
void update_display(const uint8_t left, const uint8_t right);

// The playback delay based on the petentiometer.
extern uint16_t playback_delay;


// Variables to store the segment patters.
extern uint8_t left_segment;
extern uint8_t right_segment;
/**
 * Configures SPI for display multiplexing.
 *
 * @warning - PA1 must be output enabled.
 * @warning - PC0 and PC2 must be output enabled.
 * @warning - Interrupts must be enabled globally.
 */
void spi_init(void);


/**
 * Determines the segment for digits
 * d1 is the lhs of the display
 * d2 is the rhs of the display
 * num is the number that is being dusplayed
 */
void find_digits (uint8_t num, uint8_t* d1, uint8_t* d2);

extern volatile uint8_t segs[10];
