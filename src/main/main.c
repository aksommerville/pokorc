#include "platform.h"
#include "synth.h"
#include "fakesheet.h"
#include "data.h"
#include "menu.h"
#include "game.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Globals.
 */

static uint16_t fbstorage[96*64];
static struct image fb={
  .v=fbstorage,
  .w=96,
  .h=64,
  .stride=96,
};
static struct image dancerdst;

static uint8_t input=0;
static uint8_t pvinput=0;
static uint32_t framec=0;

// If null, we are in the menu.
static const struct songinfo *songinfo=0;

static struct synth synth={0};
static struct fakesheet fakesheet={0};

/* Synthesizer.
 */

int16_t audio_next() {
  return synth_update(&synth);
}

/* Main loop.
 */
 
#if 0 // Input automation! Hacked this in to track down a bug, but leaving since I bet we'll need it again.
static uint8_t xxx_input_automation[]={
  0,//no delay on the first event
  60,BUTTON_A,
  13,0,
  255,//required final delay
};
static uint16_t xxx_input_automationp=0;
static uint8_t xxx_input_automation_delay=0;
static uint8_t xxx_input_override=0;
#endif
 
void loop() {
  framec++;

  input=platform_update();
  
  #if 0 // Input automation
  if (xxx_input_automation_delay) {
    xxx_input_automation_delay--;
    input=xxx_input_override;
  } else if (xxx_input_automationp<sizeof(xxx_input_automation)) {
    xxx_input_override=xxx_input_automation[xxx_input_automationp++];
    xxx_input_automation_delay=xxx_input_automation[xxx_input_automationp++];
  }
  #endif
  
  if (input!=pvinput) {
    if (songinfo) {
      if (!game_input(input,pvinput)) {
        songinfo=0;
        menu_init();
      }
    } else {
      songinfo=menu_input(input,pvinput);
      if (songinfo) game_begin(songinfo,&synth,&fakesheet);
    }
    pvinput=input;
  }
  
  if (songinfo) {
    game_update();
    game_render(&fb);
  } else {
    menu_update(&fb);
  }
  
  platform_send_framebuffer(fb.v);
}

/* Init.
 */

void setup() {

  int32_t audio_rate=0;
  platform_init(&audio_rate);
  synth_init(&synth,audio_rate);
  fakesheet.frames_per_tick=synth.frames_per_tick;
  
  synth.wavev[0]=wave0;
  synth.wavev[1]=wave1;
  synth.wavev[2]=wave2;
  synth.wavev[3]=wave3;
  synth.wavev[4]=wave4;
  synth.wavev[5]=wave5;
  synth.wavev[6]=wave6;
  synth.wavev[7]=wave7;
  
  menu_init();
}
