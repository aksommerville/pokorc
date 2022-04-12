#include "dancer.h"
#include "platform.h"
#include "data.h"

/* Globals.
 */
 
static uint8_t dancerid=0;

/* Mary, of Little Lamb fame.
 */
 
static void mary_init() {
}

static void mary_update(
  struct image *dst,
  uint32_t timep,uint32_t timec,
  uint32_t beatp
) {
  //TODO
}

/* The sassy raccoon.
 */
 
static void raccoon_init() {
}

static void raccoon_update(
  struct image *dst,
  uint32_t timep,uint32_t timec,
  uint32_t beatp
) {
  uint8_t frame=(timep*4)/timec;
  int16_t srcx=0;
  switch (frame) {
    case 0: srcx=12*0; break;
    case 1: srcx=12*4; break;
    case 2: srcx=12*0; break;
    case 3: srcx=12*5; break;
  }
  image_blit_colorkey(dst,6,0,&dancer,srcx,0,12,24);
}

/* Dancing robot.
 */
 
static void robot_init() {
}

static void robot_update(
  struct image *dst,
  uint32_t timep,uint32_t timec,
  uint32_t beatp
) {
  //TODO
}

/* Astronaut.
 */
 
static void astronaut_init() {
}

static void astronaut_update(
  struct image *dst,
  uint32_t timep,uint32_t timec,
  uint32_t beatp
) {
  //TODO
}

/* Dispatch.
 */
 
void dancer_init(uint8_t _dancerid) {
  dancerid=_dancerid;
  switch (dancerid) {
    case DANCER_ID_MARY: mary_init(); break;
    case DANCER_ID_RACCOON: raccoon_init(); break;
    case DANCER_ID_ROBOT: robot_init(); break;
    case DANCER_ID_ASTRONAUT: astronaut_init(); break;
  }
}
 
void dancer_update(
  struct image *dst,
  uint32_t timep,uint32_t timec,
  uint32_t beatp
) {
  if (!timec) timec=1;
  if (timep>=timec) timep=0;
  switch (dancerid) {
    case DANCER_ID_MARY: mary_update(dst,timep,timec,beatp); break;
    case DANCER_ID_RACCOON: raccoon_update(dst,timep,timec,beatp); break;
    case DANCER_ID_ROBOT: robot_update(dst,timep,timec,beatp); break;
    case DANCER_ID_ASTRONAUT: astronaut_update(dst,timep,timec,beatp); break;
  }
}
