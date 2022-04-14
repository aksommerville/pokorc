#include "menu.h"
#include "data.h"
#include "highscore.h"
#include <string.h>
#include <stdio.h>

/* Globals.
 */
 
static int8_t menup=0;
static uint8_t medalv[16];
static uint32_t scorev[16];

/* Init.
 */
 
void menu_init() {
  memset(medalv,0,sizeof(medalv));
  uint8_t i=0;
  for (;i<songinfoc;i++) {
    highscore_get(scorev+i,medalv+i,songinfov[i].songid);
  }
}

/* Move selection.
 */
 
static void menu_move(int8_t d) {
  menup+=d;
  if (menup<0) menup=songinfoc-1;
  else if (menup>=songinfoc) menup=0;
  //TODO sound effect?
}

/* Input.
 */

const struct songinfo *menu_input(uint8_t input,uint8_t pvinput) {
  #define PRESS(tag) ((input&BUTTON_##tag)&&!(pvinput&BUTTON_##tag))
  if (PRESS(A)) {
    //TODO sound effect?
    return songinfov+menup;
  }
  if (PRESS(UP)) menu_move(-1);
  if (PRESS(DOWN)) menu_move(1);
  #undef PRESS
  return 0;
}

/* Update.
 */
 
void menu_update(struct image *fb) {
  memset(fb->v,0,fb->w*fb->h*2);
  
  // List of songs, shouldn't be more than 6.
  int16_t x=9,y=0;
  uint8_t i=0;
  const uint8_t *medal=medalv;
  for (;i<songinfoc;i++,y+=9,medal++) {
    const struct songinfo *songinfo=songinfov+i;
    uint16_t color=(i==menup)?0xff07:0x1084;
    image_blit_string(fb,x,y,songinfo->name,-1,color,font);
    if ((*medal>=1)&&(*medal<=4)) {
      image_blit_colorkey(fb,0,y+1,&bits,60+((*medal)-1)*7,22,7,7);
    }
  }
  
  // Seventh row: High score for highlighted song.
  if ((menup>=0)&&(menup<16)) {
    y=9*6;
    char tmp[32];
    int16_t tmpc=snprintf(tmp,sizeof(tmp),"Hi: %d",scorev[menup]);
    if ((tmpc>0)&&(tmpc<sizeof(tmp))) {
      image_blit_string(fb,1,y,tmp,tmpc,0xffff,font);
    }
  }
}
