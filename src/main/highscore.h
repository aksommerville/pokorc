/* highscore.h
 * Access to persisted high scores file.
 * Each song has a 16-bit id, 24-bit high score, and 8-bit medal.
 */
 
#ifndef HIGHSCORE_H
#define HIGHSCORE_H

#include <stdint.h>

#define HIGHSCORE_MEDAL_NONE        0 /* Never played it. */
#define HIGHSCORE_MEDAL_TURKEY      1 /* Played with lots of mistakes. */
#define HIGHSCORE_MEDAL_PARTICIPANT 2 /* Played with few mistakes. */
#define HIGHSCORE_MEDAL_RIBBON      3 /* Played without mistakes. */
#define HIGHSCORE_MEDAL_CUP         4 /* Played perfect. */

void highscore_get(uint32_t *score,uint8_t *medal,uint16_t songid);
void highscore_set(uint16_t songid,uint32_t score,uint8_t medal);

/* Slightly different concern.
 * Send the score to our server via USB, if possible.
 */
void highscore_send(uint16_t songid,uint32_t score,uint8_t medal);

#endif
