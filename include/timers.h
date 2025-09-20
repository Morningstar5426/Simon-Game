#include <stdint.h>
extern volatile uint16_t elapsed_time;
extern volatile uint8_t pb_debounced_state;

/**
 * Updates the playback delay of the buzzer and display
 * based on the position of the petentiometer.
 */
void update_playback_delay(void);
extern uint16_t playback_delay; // Unsigned integer of the playback delay