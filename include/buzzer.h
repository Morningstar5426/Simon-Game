#include <stdint.h>

/**
 * Enables the buzzer and sets the octave of the tone
 *
 * @param tone The frequency of the sound to be played.
 */
void buzzer_on(const uint8_t tone);

// Disables the buzzer
void buzzer_off(void);
// Increases the octave of the note
void increase_octave(void);
// Decreases the octave of the note
void decrease_octave(void);
// Resets the octave 
void reset_frequency(void);
