#include <stdint.h>

/**
 * Initialses the LFSR.
 * @param seed the seed to generate the LFSR
 * @return initial LFSR state based on the seed
 */
uint32_t initialiseLFSR(uint32_t seed);

/**
 * Steps the LFSR to generate the next random value.
 * @param stateLFSR Pointer to the current LFSR state to be updated.
 * @return The next random step in the sequence.
 */
uint8_t Step(uint32_t *stateLFSR);

// Global variable for LFSR state
extern uint32_t stateLFSR;