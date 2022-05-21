#include "dancer.h"
#include "platform.h"
#include "data.h"
#include <stdlib.h>

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
  
  image_blit_colorkey(dst,6,0,&dancer,0+bodyframe*12,8,12,20);
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
 
#define STAR_COUNT 60
static struct star {
  uint32_t y; // >>16
  uint16_t luma; // brightness and speed
  int16_t x; // 0..23
} starv[STAR_COUNT];

static uint8_t astrovelocity=0;
static uint8_t astromanextent=0;
 
static void astronaut_init() {
  struct star *star=starv;
  uint8_t i=STAR_COUNT;
  for (;i-->0;star++) {
    star->x=rand()%24;
    star->y=(rand()%24)<<16;
    star->luma=rand()|0x0080;
  }
  astrovelocity=0;
  astromanextent=0;
}

static void astronaut_update(
  struct image *dst,
  uint32_t timep,uint32_t timec,
  uint32_t beatp,
  uint8_t quality
) {

  // At end of song, pack up.
  if (timec==1) quality=0;
  
  switch (quality) {
    case 0: {
        if (astrovelocity>0) astrovelocity--;
        if (astromanextent>0) astromanextent--;
      } break;
    case 1: {
        if (astrovelocity<40) astrovelocity++;
        else if (astrovelocity>40) astrovelocity--;
        if (astromanextent>0) astromanextent--;
      } break;
    case 2: {
        if (astrovelocity<120) astrovelocity++;
        else if (astrovelocity>120) astrovelocity--;
        if (astromanextent<120) astromanextent++; // both 120, it's a coincidence
      } break;
    case 3: {
        if (astrovelocity<190) astrovelocity++;
        else if (astrovelocity>190) astrovelocity--;
      } break;
    default: {
        if (astrovelocity<255) astrovelocity++;
      }
  }
  
  // Background starfield.
  struct star *star=starv;
  uint8_t i=STAR_COUNT;
  for (;i-->0;star++) {
    uint16_t rgb=((star->luma&0xf800)>>3)|0x0100;
    rgb=rgb|(rgb>>5)|(rgb>>10)|(rgb<<5);
    dst->v[(star->y>>16)*dst->stride+star->x]=rgb;
    uint16_t v=(star->luma*astrovelocity)>>8;
    star->y+=v;
    if (star->y>=(24<<16)) {
      star->x=rand()%24;
      star->y=0;
      star->luma=rand()|0x0008;
    }
  }
  
  // Ship body.
  if (astromanextent) {
    image_blit_colorkey(dst,13,5,&dancer,10,28,10,14);
  } else {
    image_blit_colorkey(dst,13,5,&dancer,0,28,10,14);
  }
  
  // Fire from engines.
  if (quality) {
    image_blit_colorkey(dst,13,19,&dancer,(beatp&1)?10:0,42,10,4);
  }
  
  if (astromanextent) {
    uint8_t mandx=(astromanextent*9)/120;
    uint8_t mandy=(astromanextent*6)/120;
    int8_t truncate=8-mandx;
    if (truncate<0) truncate=0;
    
    uint8_t manframe=0;
    if (astromanextent>=120) {
      if (timep>=(timec>>1)) {
        manframe=(beatp&1)?1:2;
      }
    }
  
    // Umbilicus.
    image_blit_colorkey(dst,10+truncate,8+truncate,&dancer,20+truncate,28+truncate,7-truncate,7-truncate);
  
    // Space man.
    image_blit_colorkey(dst,12-mandx,9-mandy,&dancer,20+8*manframe,35,8,9);
  }
}

/* Elf.
 */
 
static void elf_init() {
}

static void elf_update(
  struct image *dst,
  uint32_t timep,uint32_t timec,
  uint32_t beatp,
  uint8_t quality,
  uint8_t notec
) {
  int frame;
  if ((timec<=1)||(quality<1)) {
    frame=0;
  } else {
    frame=(beatp&1)<<1;
    if (timep>=timec>>1) frame++;
  }
  
  // Toot instead.
  if (quality>=3) {
    image_blit_colorkey(dst,3,1,&dancer,0,46,12,11);
    switch (frame) {
      case 0:
      case 2: image_blit_colorkey(dst,4,10,&dancer,42,55,14,13); break;
      case 1: image_blit_colorkey(dst,4,10,&dancer,56,55,14,13); break;
      case 3: image_blit_colorkey(dst,4,10,&dancer,70,55,14,13); break;
    }
    if (notec) {
      image_blit_colorkey(dst,18,9,&dancer,18,46,4,7);
    }
    return;
  }
  
  // Head.
  if (frame>=2) {
    image_blit_colorkey_flop(dst,7,1,&dancer,0,46,12,11);
  } else {
    image_blit_colorkey(dst,3,1,&dancer,0,46,12,11);
  }
  
  // Body.
  switch (frame) {
    case 0:
    case 2: image_blit_colorkey(dst,4,12,&dancer,0,57,14,11); break;
    case 1: image_blit_colorkey(dst,4,8,&dancer,14,53,14,15); break;
    case 3: image_blit_colorkey(dst,4,8,&dancer,28,53,14,15); break;
  }
}

/* Ghost.
 */
 
static void ghost_init() {
}

static void ghost_update(
  struct image *dst,
  uint32_t timep,uint32_t timec,
  uint32_t beatp,
  uint8_t quality
) {
  int frame;
  if ((timec<=1)||(quality<1)) {
    frame=0;
  } else {
    frame=(beatp&1)<<1;
    if (timep>=timec>>1) frame++;
  }
  
  // Body.
  image_blit_colorkey(dst,0,0,&dancer,frame*24,68,24,24);
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
    case DANCER_ID_ELF: elf_init(); break;
  }
}
 
void dancer_update(
  struct image *dst,
  uint32_t timep,uint32_t timec,
  uint32_t beatp,
  uint8_t quality,
  uint8_t notec
) {
  if (!timec) timec=1;
  if (timep>=timec) timep=0;
  switch (dancerid) {
    case DANCER_ID_MARY: mary_update(dst,timep,timec,beatp,quality); break;
    case DANCER_ID_RACCOON: raccoon_update(dst,timep,timec,beatp,quality); break;
    case DANCER_ID_ROBOT: robot_update(dst,timep,timec,beatp,quality); break;
    case DANCER_ID_ASTRONAUT: astronaut_update(dst,timep,timec,beatp,quality); break;
    case DANCER_ID_ELF: elf_update(dst,timep,timec,beatp,quality,notec); break;
    case DANCER_ID_GHOST: ghost_update(dst,timep,timec,beatp,quality); break;
  }
}
