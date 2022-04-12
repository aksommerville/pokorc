/* menu.h
 * Everything around the intro splash (song select).
 */
 
#ifndef MENU_H
#define MENU_H

#include "platform.h"

struct songinfo;

void menu_init();

// Returns a songinfo when one is selected -- stop calling menu at that point.
const struct songinfo *menu_input(uint8_t input,uint8_t pvinput);

void menu_update(struct image *fb);

#endif
