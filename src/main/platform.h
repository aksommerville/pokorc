#ifndef PLATFORM_H
#define PLATFORM_H

#define BUTTON_LEFT     0x01
#define BUTTON_RIGHT    0x02
#define BUTTON_UP       0x04
#define BUTTON_DOWN     0x08
#define BUTTON_A        0x10
#define BUTTON_B        0x20

#include <stdint.h>

#ifdef __cplusplus
  extern "C" {
#endif

/* Provided by game; called by Arduino or genioc.
 *********************************************************************/

void setup();
void loop();
int16_t audio_next();

/* Provided by driver.
 *********************************************************************/
 
uint8_t platform_init();
uint8_t platform_update();
void platform_send_framebuffer(const void *fb);

/* Imaging.
 *********************************************************************/
 
struct image {
  uint16_t *v;
  int w,h;
};

/* We check output bounds but not input -- one presumes you know the input geometry well.
 */
void image_blit_opaque(
  struct image *dst,int16_t dstx,int16_t dsty,
  const struct image *src,int16_t srcx,int16_t srcy,
  int16_t w,int16_t h
);
void image_blit_colorkey(
  struct image *dst,int16_t dstx,int16_t dsty,
  const struct image *src,int16_t srcx,int16_t srcy,
  int16_t w,int16_t h
);

#ifdef __cplusplus
  }
#endif

#endif
