#include "dancer.h"
#include "platform.h"
#include "data.h"

/* Globals.
 */
 
static uint8_t dancerid=0;
static uint8_t disappointment=0;

/* Mary, of Little Lamb fame.
 */
 
static void mary_init() {
  disappointment=0;
}

static void mary_update(
  struct image *dst,
  uint32_t timep,uint32_t timec,
  uint32_t beatp,
  uint8_t quality
) {
  uint8_t bodyframe=0;
  uint8_t headframe=0;
  uint8_t headdy=0;
  uint8_t larmframe=0;
  uint8_t rarmframe=0;
  int8_t rarmdy=0;
  
  if (quality) {
    headdy=(beatp&1);
    switch ((timep*4)/timec) {
      case 0: bodyframe=0; rarmdy=0; break;
      case 1: bodyframe=1; rarmdy=-1; break;
      case 2: bodyframe=0; rarmdy=0; break;
      case 3: bodyframe=2; rarmdy=1; break;
    }
  
    switch (beatp&1) {
      case 0: switch ((timep*2)/timec) {
          case 0: larmframe=0; break;
          case 1: larmframe=1; break;
        } break;
      case 1: switch ((timep*2)/timec) {
          case 0: larmframe=0; break;
          case 1: larmframe=2; break;
        } break;
    }
    
    disappointment=60; // we're good now, so if a future frame is bad, be disappointed
    
  } else if (disappointment) {
    disappointment--;
    headframe=1;
    
  // Zero quality, check your email.
  } else {
    headframe=2;
    switch ((timep*3)/timec) {
      case 0: rarmframe=1; break;
      case 1: rarmframe=2; break;
      case 2: rarmframe=3; break;
    }
  }
  
  switch (rarmframe) {
    case 0: image_blit_colorkey(dst,13,8+rarmdy,&dancer,65,26,8,15); break;
    case 1: image_blit_colorkey(dst,13,8,&dancer,73,26,6,15); break;
    case 2: image_blit_colorkey(dst,13,8,&dancer,79,26,6,15); break;
    case 3: image_blit_colorkey(dst,13,8,&dancer,85,26,6,15); break;
  }
  image_blit_colorkey(dst,2,10,&dancer,48+bodyframe*16,12,16,14);
  image_blit_colorkey(dst,3,headdy,&dancer,48+headframe*14,0,14,12);
  switch (larmframe) {
    case 0: image_blit_colorkey(dst,3,11,&dancer,48,26,4,9); break;
    case 1: image_blit_colorkey(dst,0,11,&dancer,52,26,6,8); break;
    case 2: image_blit_colorkey(dst,3,11,&dancer,58,26,7,7); break;
  }
}

/* The sassy raccoon.
 */
 
static void raccoon_init() {
}

static void raccoon_update(
  struct image *dst,
  uint32_t timep,uint32_t timec,
  uint32_t beatp,
  uint8_t quality
) {
  uint8_t headframe=0; // 0,1,2 = neutral,blink,hoot
  uint8_t bodyframe=0; // 0,1,2,3 = neutral,tap,disco1,disco2
  int8_t headdy=0;
  
  // Poor quality. Just stand there and stare at the user.
  if (quality<1) {
    headframe=0;
    bodyframe=0;
    
  // Getting warmed up. Tap your foot and blink a little.
  } else if (quality<2) {
    if ((beatp&1)&&(timep<3000)) headframe=1;
    else headframe=0;
    bodyframe=(timep*2)/timec;
    
  // High quality, dance like you mean it.
  } else {
    if (!(beatp&3)) headframe=2;
    else headframe=0;
    switch ((timep*4)/timec) {
      case 0: bodyframe=0; break;
      case 1: bodyframe=2; headdy=-1; break;
      case 2: bodyframe=0; break;
      case 3: bodyframe=3; headdy=1; break;
    }
  }
  
  image_blit_colorkey(dst,6,0,&dancer,0+bodyframe*12,8,12,24);
  image_blit_colorkey(dst,7,1+headdy,&dancer,0+headframe*10,0,10,8);
}

/* Dancing robot.
 */
 
static void robot_init() {
}

static void robot_update(
  struct image *dst,
  uint32_t timep,uint32_t timec,
  uint32_t beatp,
  uint8_t quality
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
  uint32_t beatp,
  uint8_t quality
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
  uint32_t beatp,
  uint8_t quality
) {
  if (!timec) timec=1;
  if (timep>=timec) timep=0;
  switch (dancerid) {
    case DANCER_ID_MARY: mary_update(dst,timep,timec,beatp,quality); break;
    case DANCER_ID_RACCOON: raccoon_update(dst,timep,timec,beatp,quality); break;
    case DANCER_ID_ROBOT: robot_update(dst,timep,timec,beatp,quality); break;
    case DANCER_ID_ASTRONAUT: astronaut_update(dst,timep,timec,beatp,quality); break;
  }
}
