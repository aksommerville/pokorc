/* data.h
 * References and helpers around the embedded data.
 */
 
#ifndef DATA_H
#define DATA_H

#include "platform.h"

extern struct image bits;
extern struct image dancer;

extern const int16_t wave0[];
extern const int16_t wave1[];
extern const int16_t wave2[];
extern const int16_t wave3[];
extern const int16_t wave4[];
extern const int16_t wave5[];
extern const int16_t wave6[];
extern const int16_t wave7[];

extern const uint8_t inversegamma[];
extern const uint8_t carribean[];
extern const uint8_t goashuffle[];
extern const uint8_t goatpotion[];
extern const uint8_t mousechief[];
extern const uint8_t tinylamb[];

extern const struct songinfo {
  const uint8_t *song;
  const char *name;
  uint8_t dancerid;
} songinfov[];
extern const uint8_t songinfoc;

extern const uint32_t font[96];

#endif
