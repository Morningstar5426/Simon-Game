#include "lfsr.h"
#include <stdint.h>
#define MASK 0xE2024CAB


uint32_t initialiseLFSR (uint32_t studentNumber) {
    return studentNumber;
}

// Generates the next step in the sequence
uint8_t Step (uint32_t *stateLFSR) {
    uint8_t bit = *stateLFSR & 1;
    *stateLFSR >>= 1;
    if (bit) {
        *stateLFSR ^= MASK;
    }
    return *stateLFSR & 0b11;
}
