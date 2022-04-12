/* game.h
 * Game logic and rendering here.
 * We also handle the "game over" splash, but not the "song select" one.
 */
 
#ifndef GAME_H
#define GAME_H

#include <stdint.h>

struct songinfo;
struct image;
struct synth;
struct fakesheet;

void game_begin(
  const struct songinfo *songinfo,
  struct synth *synth,
  struct fakesheet *fakesheet
);

// 1 to proceed, 0 to return to song select.
uint8_t game_input(uint8_t input,uint8_t pvinput);

void game_update();
void game_render(struct image *image);

#endif
