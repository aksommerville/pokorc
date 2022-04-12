/* game.h
 * Game logic and rendering here.
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

void game_input(uint8_t input,uint8_t pvinput);
void game_update();
void game_render(struct image *image);

#endif
