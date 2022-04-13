/* dancer.h
 * The little animated figure that dances with the music.
 */
 
#ifndef DANCER_H
#define DANCER_H

struct image;

#include <stdint.h>

#define DANCER_ID_MARY      1
#define DANCER_ID_RACCOON   2
#define DANCER_ID_ROBOT     3
#define DANCER_ID_ASTRONAUT 4

void dancer_init(uint8_t dancerid);

/* Draw to (dst), filling it.
 * (timep) should be in (0..timec-1), our position within the beat.
 * (beatp) is how many beats elapsed.
 * (quality) comes mostly from the running combo: 0..4
 */
void dancer_update(
  struct image *dst,
  uint32_t timep,uint32_t timec,
  uint32_t beatp,
  uint8_t quality
);

#endif
