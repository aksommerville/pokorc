#include "platform.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Globals.
 */

extern struct image bits;

static uint16_t fbstorage[96*64];
static struct image fb={
  .v=fbstorage,
  .w=96,
  .h=64,
};

static uint8_t pvinput=0;

/* TODO Synthesizer.
 */
 
static int16_t halfperiod=100;
static int16_t level=10000;
static int16_t p=0;

int16_t audio_next() {
  if (!p--) {
    p=halfperiod;
    level=-level;
  }
  return level;
}

/* Notes XXX TEMP get this organized
 */
 
#define NOTEC 8
 
static struct note {
  uint8_t col; // 0..4: both horz position and icon
  int8_t y; // 0..63 (or maybe a bit further); vertical midpoint
  uint8_t scored;
} notev[NOTEC];

static void init_notes() {//XXX they can init to zero in real life
  struct note *note=notev;
  uint8_t i=NOTEC;
  for (;i-->0;note++) {
    note->col=rand()%5;
    note->y=rand()%64; // don't let them be the same initially; they move at the same rate
    note->scored=0;
  }
}

/* Consider the song's current time.
 * Create new notes if warranted, and reposition all the ones we have.
 */

static void update_notes() {
  //XXX TEMP no song yet, so just move everything down at 1 px/frame and replace randomly when they go offscreen.
  struct note *note=notev;
  uint8_t i=NOTEC;
  for (;i-->0;note++) {
    note->y++;
    if (note->y>64+5) {
      note->col=rand()%5;
      note->y=-5; // just offscreen, mind that it must go 2 pixels onscreen to clear the frame, so it's really 3 off
      note->scored=0;
    }
  }
}

/* A key was pressed.
 * Find the nearest unscored note in that column and score it, or register a botch.
 */
 
#define DISTANCE_LIMIT 10 /* pixels */

static void play_note(uint8_t col) {
  struct note *best=0;
  struct note *note=notev;
  uint8_t bestdistance=DISTANCE_LIMIT;
  uint8_t i=NOTEC;
  for (;i-->0;note++) {
    if (note->scored) continue;
    if (note->col!=col) continue;
    int8_t distance=note->y-52; // The "now" line is at y==52
    if (distance<0) distance=-distance;
    if (distance>bestdistance) continue;
    best=note;
    bestdistance=distance;
  }
  //TODO make a sound
  //TODO fireworks or something
  if (best) {
    int8_t score=DISTANCE_LIMIT-bestdistance;
    //fprintf(stderr,"+%d\n",score);
    best->scored=1;
  } else {
    //fprintf(stderr,"BOTCH!\n");
  }
}

/* Main loop.
 */
 
void loop() {

  uint8_t input=platform_update();
  if (input!=pvinput) {
    #define PRESS(tag) ((input&BUTTON_##tag)&&!(pvinput&BUTTON_##tag))
         if (PRESS(LEFT)) play_note(0);
    else if (PRESS(UP)) play_note(1);
    else if (PRESS(RIGHT)) play_note(2);
    else if (PRESS(B)) play_note(3);
    else if (PRESS(A)) play_note(4);
    #undef DOWN
    pvinput=input;
  }
  
  update_notes();
  
  // Black out.
  memset(fbstorage,0,sizeof(fbstorage));
  
  // 5 track backgrounds.
  image_blit_opaque(&fb, 8,0,&bits,7,7,2,64);
  image_blit_opaque(&fb,20,0,&bits,7,7,2,64);
  image_blit_opaque(&fb,32,0,&bits,7,7,2,64);
  image_blit_opaque(&fb,44,0,&bits,7,7,2,64);
  image_blit_opaque(&fb,56,0,&bits,7,7,2,64);
  
  // Notes.
  {
    struct note *note=notev;
    uint8_t i=NOTEC;
    for (;i-->0;note++) {
      if (note->col>4) continue; // Can use this as "none" indicator.
      int16_t srcy=note->scored?31:22;
      image_blit_colorkey(&fb,4+note->col*12,note->y-4,&bits,9+note->col*10,srcy,10,9);
    }
  }
  
  // "Now" line.
  image_blit_colorkey(&fb,0,52,&bits,10,21,66,1);
  
  // 5 button indicators.
  #define BTN(ix,tag) image_blit_colorkey(&fb,3+ix*12,56,&bits,9+ix*12,7+((input&BUTTON_##tag)?7:0),12,7);
  BTN(0,LEFT)
  BTN(1,UP)
  BTN(2,RIGHT)
  BTN(3,B)
  BTN(4,A)
  #undef BTN
  
  // Frames. (note that the corners overlap, that's ok)
  image_blit_colorkey(&fb,0,0,&bits,3,7,4,64); // left
  image_blit_colorkey(&fb,92,0,&bits,0,7,4,64); // right
  image_blit_colorkey(&fb,0,0,&bits,0,3,96,4); // top
  image_blit_colorkey(&fb,0,60,&bits,0,0,96,4); // bottom
  image_blit_colorkey(&fb,62,0,&bits,0,7,7,64); // full-height horz divider

  platform_send_framebuffer(fb.v);
}

/* Init.
 */

void setup() {
  platform_init();
  init_notes();
}
