
#include <stdio.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include "lfsr.h"
#include "uart.h"
#include "display_macros.h"
#include "initialisation.h"
#include "buzzer.h"
#include "display.h"
#include "high_score.h"
#include "timers.h"
#define MAX_SEQUENCE_LENGTH 1000
// Initialising variables
volatile uint16_t elapsed_time;
uint32_t studentNumber = 0x11548151;
uint32_t stateLFSR;
uint8_t sequence_length = 1;
extern volatile uint8_t reset_flag;


uint8_t sequence[MAX_SEQUENCE_LENGTH];  // Array to store the sequence
uint8_t current_sequence_length = 0;
uint8_t current_step = 0; 

void update_name_timeout(uint16_t elapsed_time);

void state_machine(void);

int main() {
    // Generates interrupts for the initialised hardware
    cli();
    adc_init();
    buttons_init();
    spi_init();
    pwm_init();
    timer_init();
    uart_init();
    sei();
    stateLFSR = initialiseLFSR(studentNumber); // Initialises the stateLFSR
    state_machine();
}

// Handles the pressing of buttons to their respective buzzer frequency and display patttern
void enable_outputs(uint8_t value)
{
    if (value == 0)
    {
        update_display((DISP_SEG_E & DISP_SEG_F), DISP_OFF);
        buzzer_on(0);
    }
    else if (value == 1)
    {
        update_display((DISP_SEG_B & DISP_SEG_C), DISP_OFF);
        buzzer_on(1);
    }
    else if (value == 2)
    {
        update_display(DISP_OFF, (DISP_SEG_E & DISP_SEG_F));
        buzzer_on(2);
    }
    else if (value == 3)
    {
        update_display(DISP_OFF, (DISP_SEG_B & DISP_SEG_C));
        buzzer_on(3);
    }
    elapsed_time = 0;

}

// Disables the outputs
void disable_outputs(void)
{
    update_display(DISP_OFF, DISP_OFF);
    buzzer_off();
}




// State for handling the Simon game
typedef enum
{
    GENERATE,
    INPUT_WAITING,
    INPUT_RECEIVED,
    INPUT_EVALUATE,

    DISPLAY_SUCCESS,
    DISPLAY_FAILURE,
    DISPLAY_SCORE
} state_t;


void state_machine(void)
{
    disable_outputs();

    
    stateLFSR = initialiseLFSR(studentNumber); // Initialises the stateLFSR in the state machine
    // Player input
    uint8_t input = 0;
    // Player reaction time
    // Initialising the button handling
    uint8_t pb_previous_state, pb_current_state = 0xFF;
    uint8_t pb_falling_edge, pb_rising_edge = 0x00;
    uint8_t pb_released = 0;
    state_t my_state = GENERATE; // Sets the initial state
    uint8_t next_step;
    static uint8_t game_over = 0; // Handles printing the game over statement

    


    while (1)
    {
        update_name_timeout(elapsed_time); // Checks that the uart entering of name is less than 5s
        // Debouncing handling
        pb_previous_state = pb_current_state;
        pb_current_state = pb_debounced_state;
        pb_falling_edge = (pb_previous_state ^ pb_current_state) & pb_previous_state;
        pb_rising_edge = (pb_previous_state ^ pb_current_state) & pb_current_state;

        switch (my_state)
        {
            // Generates the pattern
            case GENERATE:
                game_over = 0;
                update_playback_delay();
                if (current_sequence_length < MAX_SEQUENCE_LENGTH) {
                    next_step = Step(&stateLFSR);
                    sequence[current_sequence_length++] = next_step;  // Store the new step
                }

                // Replay the previous steps
                for (uint8_t i = 0; i < current_sequence_length; i++) {
                    enable_outputs(sequence[i]); // Display the previous steps
                    elapsed_time = 0;
                    while (elapsed_time  < playback_delay >> 1) {}
                    disable_outputs();
                    elapsed_time = 0;
                    while (elapsed_time < playback_delay >> 1) {}
                }
                elapsed_time = 0; // Resets timer
                my_state = INPUT_WAITING; // Changes state
                break;

        

            case INPUT_WAITING:
                disable_outputs();
            // check if any of these buttons have been presssed
            // use falling edge and a combined bit mask for all buttons
                if ((pb_falling_edge & 0xF0) || (uart_input != 4)) {
                    pb_released = 0;
                    
                    if (pb_falling_edge & PIN4_bm) {
                        input = 0;
                    }
                    else if (pb_falling_edge & PIN5_bm) {
                        input = 1;
                    }
                    else if (pb_falling_edge & PIN6_bm) {
                        input = 2;
                    }
                    else if (pb_falling_edge & PIN7_bm) {
                        input = 3;
                        
                    }
                    else {
                        input = uart_input;
                        uart_input = 4;
                        pb_released = 1;
                    }
                    // check each individual button and update input
                    enable_outputs(input);
                    elapsed_time = 0;
                    my_state = INPUT_RECEIVED;
                    
                }
                break;

            case INPUT_RECEIVED:
                // check if any buttons have been released (rising edge)
                // disable outputs
                if (!pb_released) {
                    if (pb_rising_edge & 0xF0) {
                        pb_released = 1;
                    }
                }
                else {
                    if (elapsed_time > (playback_delay >>1)) {
                        disable_outputs();
                        my_state = INPUT_EVALUATE;
                    }
                }
                break;

            case INPUT_EVALUATE:
                if (input == sequence[current_step]) {
                    update_display(DISP_ON, DISP_ON);
                    current_step++; // Increments the current_step
                    elapsed_time = 0;

                    if (current_step == current_sequence_length) {
                        // User matched the whole sequence
                        sequence_length++;  // Increments the score
                        elapsed_time = 0;
                        my_state = DISPLAY_SUCCESS;
                        current_step = 0;   // Reset for the next sequence
                    } 
                    else {
                        elapsed_time = 0;
                        my_state = INPUT_WAITING; // Wait for the next input
                    }
                } else {
                    elapsed_time = 0;
                    my_state = DISPLAY_FAILURE; // User failed to match the sequence
                }
                break;

            case DISPLAY_SUCCESS:
                if (elapsed_time == 0) {
                    // Send success message and score via UART when entering this state.
                    printf("SUCCESS\n");
                    printf("%d\n", sequence_length - 1);
                }
                // Turns off the outputs after playback_delay time has been reached
                if (elapsed_time >= playback_delay) {
                    disable_outputs();
                    my_state = GENERATE;
                    elapsed_time = 0;
                }
                break;

            case DISPLAY_FAILURE:
                
                // Displays the failure pattern for the length of playback delay
                if (elapsed_time <= playback_delay) {
                    update_display(DISP_SEG_G, DISP_SEG_G);
                    if (!game_over) {
                    // Prints the game over and score statements
                    printf("GAME OVER\n");
                    printf("%d\n", sequence_length);
                    game_over = 1;
                    elapsed_time = 0;
                    }
                }
                
                else {
                    // Handles the displaying of the user score on failure
                    
                    uint8_t tens, ones;
                    find_digits(sequence_length % 100, &tens, &ones);
                    elapsed_time = 0; 
                    my_state = DISPLAY_SCORE;
                    
                }
                break;
            
            case DISPLAY_SCORE:
            // Displays the score for length of playback delay
            if (elapsed_time <= playback_delay) {
                update_display(left_segment, right_segment);
                // Keep the score displayed for the playback time
            } 
            // Turns display off after the score for the length of playback delay
            else if (elapsed_time <= 2 * playback_delay) {
                display_off();
            } 
            else {
                // Starts a new game after the display has been off for the playback delay
                display_off();
                check_high_scores(sequence_length);
                sequence_length = 1;
                elapsed_time = 0;
                current_sequence_length = 0;
                current_step = 0;
                my_state = GENERATE;
            }
            break;


            // The default state of the game
            default:
                my_state = GENERATE;
                initialiseLFSR(studentNumber);
                disable_outputs();
                break;
        }


    }
}