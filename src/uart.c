#include "uart.h"
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include "buzzer.h"
#include "lfsr.h"
#include <stdio.h>
#include "high_score.h"

extern volatile uint16_t elapsed_time;
extern uint32_t studentNumber;
extern volatile uint8_t reset_flag; 
volatile uint8_t uart_input = 4;
volatile uint8_t reset_flag = 0;
static uint32_t seed_value = 0;
static uint8_t chars_received = 0;
static uint8_t seed_valid = 1;
void display_high_scores(void);

// Creates the enum of uart states
typedef enum {
    AWAITING_COMMAND,
    AWAITING_PAYLOAD,
    AWAITING_NAME_INPUT
} Serial_State;

static Serial_State SERIAL_STATE = AWAITING_COMMAND; // Sets the initial state

void reset_game(void) {
    // Resets the game variables
    sequence_length = 1;
    elapsed_time = 0;
    uart_input = 4;
    reset_flag = 0;
    current_sequence_length = 0;
    current_step = 0;
    stateLFSR = initialiseLFSR(studentNumber);
}

// Handles conversion of hex to int for seed entering
static uint8_t hexchar_to_int(char c) {
    if ('0' <= c && c <= '9')
        return c - '0';
    else if ('a' <= c && c <= 'f')
        return 10 + c - 'a';
    else
        return 16; // Invalid character
}

// Changes the game state to prompt for the user's name
void get_player_name() {
    SERIAL_STATE = AWAITING_NAME_INPUT;
    chars_received = 0;
    printf("Enter name: \n");
    uart_input = 4;
}

// Interrupt to handle uart input
ISR(USART0_RXC_vect) {
    char rx_data = USART0.RXDATAL;
    
    // Handles the uart input cases
    switch (SERIAL_STATE) {
        case AWAITING_COMMAND:
            switch (rx_data) {
                case '1':
                case 'q':
                    uart_input = 0;
                    break;
                case '2':
                case 'w':
                    uart_input = 1;
                    break;
                case '3':
                case 'e':
                    uart_input = 2;
                    break;
                case '4':
                case 'r':
                    uart_input = 3;
                    break;
                case ',':
                case 'k':
                    increase_octave();
                    break;
                case '.':
                case 'l':
                    decrease_octave();
                    break;
                case '0':
                case 'p':
                // Resets the game state whilst keeping the high scores
                    reset_frequency();
                    reset_flag = 1;
                    reset_game();
                    break;
                case '9':
                case 'o':
                // Changes the LFSR seed based on uart input
                    seed_valid = 1;
                    chars_received = 0;
                    seed_value = 0;
                    SERIAL_STATE = AWAITING_PAYLOAD;
                    printf("Enter 8-character hex SEED:\n");
                    break;
                default:
                    uart_input = 4;
                    break;
            }
            break;

        case AWAITING_PAYLOAD: {
            // Handles the change in seed
            uint8_t parsed_result = hexchar_to_int(rx_data);

            if (parsed_result != 16) {
                seed_value = (seed_value << 4) | parsed_result;
            } else {
                seed_valid = 0;
            }

            if (++chars_received == 8) {
                if (seed_valid) {
                    stateLFSR = initialiseLFSR(seed_value);
                    printf("Seed set to: 0x%08lx\n", seed_value);
                } else {
                    printf("Invalid SEED\n");
                }
                SERIAL_STATE = AWAITING_COMMAND;
            }
            break;
        }
        // If the score is a high score it prompts the user to enter a name
        case AWAITING_NAME_INPUT: {
            if (chars_received < MAX_NAME_LENGTH - 1) {
                if (rx_data == '\n' || rx_data == '\r') {
                    high_scores[current_high_score_index].name[chars_received] = '\0';
                    if (chars_received > 0) {
                        display_high_scores();
                    }
                    SERIAL_STATE = AWAITING_COMMAND; 
                    chars_received = 0;
                    return;
                }
                if (rx_data >= ' ' && rx_data <= '~') {
                    high_scores[current_high_score_index].name[chars_received++] = rx_data;
                }
            }
            // For truncating names longer than 20 chars
            else {
                if (rx_data == '\n' || rx_data == '\r') {
                    high_scores[current_high_score_index].name[MAX_NAME_LENGTH - 1] = '\0';
                    display_high_scores();
                    SERIAL_STATE = AWAITING_COMMAND; 
                    chars_received = 0;
                    return;
                }
            }
            break;
        }

        default:
            SERIAL_STATE = AWAITING_COMMAND;
            break;
    }
}
// Sends the score over uart
void display_high_scores() {
    for (uint8_t i = 0; i < MAX_HIGH_SCORES; i++) {
        if (high_scores[i].score > 0) {
            printf("%s %u\n", high_scores[i].name, high_scores[i].score);
        } 
    }
}
// Checks if the current score should be included in the high score table
void check_high_scores(uint8_t score) {
    if (score > high_scores[MAX_HIGH_SCORES - 1].score) {
        for (uint8_t i = 0; i < MAX_HIGH_SCORES; i++) {
            if (score > high_scores[i].score) {
                for (uint8_t j = MAX_HIGH_SCORES - 1; j > i; j--) {
                    high_scores[j] = high_scores[j - 1];
                }
                high_scores[i].score = score;
                current_high_score_index = i;
                get_player_name();
                return;
            }
        }
    }
}
// Checks if any inputs are entered over uart for the name within 5s,
// if not, the name is set to the currently entered characters or an empty string 
void update_name_timeout(uint16_t elapsed_time) {
    if (SERIAL_STATE == AWAITING_NAME_INPUT) {
        if (elapsed_time > 5000) {
            if (chars_received > 0) {
                high_scores[current_high_score_index].name[chars_received] = '\0';
            } else {
                high_scores[current_high_score_index].name[0] = '\0';
            }
            display_high_scores();
            SERIAL_STATE = AWAITING_COMMAND;
            chars_received = 0;
        }
    }
}