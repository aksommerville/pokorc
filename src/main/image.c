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

/* Blit from 1-bit glyph.
 */
 
uint8_t image_blit_glyph(
  struct image *dst,int16_t dstx,int16_t dsty,
  uint32_t glyph,uint16_t color
) {
  if (!glyph) return 4; // space. no output but do advance horizontally
  uint8_t srcy=glyph>>30;
  uint8_t srcw=(glyph>>27)&7;
  if (!srcw) return 0; // nonzero but zero width denotes a "nothing" glyph (which we don't use)
  dsty+=srcy;
  
  uint16_t *dstrow=dst->v+dsty*dst->w+dstx;
  uint32_t mask=0x04000000;
  uint8_t yi=8;
  for (;yi-->0;dstrow+=dst->w,dsty++) {
    if (dsty>=dst->h) return srcw+1;
    if (dsty<0) {
      mask>>=srcw;
      continue;
    }
    uint16_t *dstp=dstrow;
    uint8_t xi=srcw;
    for (;xi-->0;dstp++,mask>>=1) {
      if (!mask) return srcw+1;
      if (glyph&mask) {
        if ((dstx>=0)&&(dstx<dst->w)) {
          *dstp=color;
        }
      }
    }
  }
  return srcw+1;
}

/* Blit string (convenience).
 */
 
uint16_t image_blit_string(
  struct image *dst,int16_t dstx,int16_t dsty,
  const char *src,int8_t srcc,uint16_t color,
  const uint32_t *font
) {
  if (!src) return 0;
  if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  int16_t dstx0=dstx;
  for (;srcc-->0;src++) {
    if ((*src<0x20)||(*src>0x7e)) continue;
    dstx+=image_blit_glyph(dst,dstx,dsty,font[(*src)-0x20],color);
  }
  return dstx-dstx0;
}
