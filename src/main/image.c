#include "platform.h"
#include <string.h>

/* Blit preamble.
 */
 
#define PREBLIT \
  if (!dst||!src) return; \
  if (dstx<0) { srcx-=dstx; w+=dstx; dstx=0; } \
  if (dsty<0) { srcy-=dsty; h+=dsty; dsty=0; } \
  if (dstx>dst->w-w) w=dst->w-dstx; \
  if (dsty>dst->h-h) h=dst->h-dsty; \
  if ((w<1)||(h<1)) return;

/* Blit opaque.
 */
 
void image_blit_opaque(
  struct image *dst,int16_t dstx,int16_t dsty,
  const struct image *src,int16_t srcx,int16_t srcy,
  int16_t w,int16_t h
) {
  PREBLIT
  uint16_t *dstrow=dst->v+dsty*dst->w+dstx;
  const uint16_t *srcrow=src->v+srcy*src->w+srcx;
  int yi=h;
  int cpc=w<<1;
  for (;yi-->0;dstrow+=dst->w,srcrow+=src->w) {
    memcpy(dstrow,srcrow,cpc);
  }
}

/* Blit with colorkey.
 */
 
void image_blit_colorkey(
  struct image *dst,int16_t dstx,int16_t dsty,
  const struct image *src,int16_t srcx,int16_t srcy,
  int16_t w,int16_t h
) {
  PREBLIT
  uint16_t *dstrow=dst->v+dsty*dst->w+dstx;
  const uint16_t *srcrow=src->v+srcy*src->w+srcx;
  int yi=h;
  for (;yi-->0;dstrow+=dst->w,srcrow+=src->w) {
    uint16_t *dstp=dstrow;
    const uint16_t *srcp=srcrow;
    int xi=w;
    for (;xi-->0;dstp++,srcp++) {
      if (*srcp) *dstp=*srcp;
    }
  }
}
