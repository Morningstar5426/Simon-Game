#include <stdint.h>
#define MAX_HIGH_SCORES 5
#define MAX_NAME_LENGTH 21   //Including null-terminate


typedef struct {
    char name[21];
    uint8_t score;
} HighScore;

extern HighScore high_scores[MAX_HIGH_SCORES];
extern uint8_t current_high_score_index;

/**
 * Checks if the current score is within the range to
 * be a high score
 */
void check_high_scores(uint8_t score);