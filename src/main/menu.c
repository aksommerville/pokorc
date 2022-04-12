#include "menu.h"
#include "data.h"
#include <string.h>

/* Globals.
 */
 
static int8_t menup=0;

/* Init.
 */
 
void menu_init() {
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
  //image_blit_string(fb,5,5,"Hello cruel world!",-1,0xffff,font);
  int16_t x=1,y=0;
  uint8_t i=0;
  for (;i<songinfoc;i++,y+=9) {
    const struct songinfo *songinfo=songinfov+i;
    uint16_t color=(i==menup)?0xff07:0x1084;
    image_blit_string(fb,x,y,songinfo->name,-1,color,font);
  }
}
