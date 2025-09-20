#include "buzzer.h"

#include <avr/io.h>
#include <stdint.h>

#define TONE1_PER 4931
#define TONE2_PER 5869
#define TONE3_PER 3696
#define TONE4_PER 9862

static uint8_t selected_tone = 0;
static int8_t octave = 0;
static uint8_t buzzer_playing;
extern uint32_t stateLFSR;
extern uint8_t sequence_length;

void buzzer_on(const uint8_t tone)
{
    static const uint16_t base_periods[4] = {TONE1_PER, TONE2_PER, TONE3_PER, TONE4_PER};
    uint16_t period = base_periods[tone];
    buzzer_playing = 1;
    selected_tone = tone;
    
    if (octave>0) {
        period >>= octave;
    }
    else {
        period <<= -octave;
    }
    TCA0.SINGLE.PERBUF = period;
    TCA0.SINGLE.CMP0BUF = period >> 1;


}
void buzzer_off(void)
{
    TCA0.SINGLE.CMP0BUF = 0;
    buzzer_playing = 0;

}

void increase_octave(void)
{
    if (octave < 2) {
        octave++;
        if (buzzer_playing) {
            buzzer_on(selected_tone);
        }
    }
}
void decrease_octave(void)
{
    if (octave > -2) {
        octave--;
        if (buzzer_playing) {
            buzzer_on(selected_tone);
        }
    }
}

void reset_frequency(void) {
    octave = 0;
}


