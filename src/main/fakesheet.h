/* fakesheet.h
 * Along with each song is a "fakesheet" containing the missing notes that the player must supply.
 * It's formatted a little different from song, because it must describe the input channel (button), and must be future-peekable.
 * (for you non-musical programmers: A "fakesheet" is a scrappy bit of written music to guide improvisation, big in jazz).
 */
 
#ifndef FAKESHEET_H
#define FAKESHEET_H

#include <stdint.h>

/* event:
 *   0xffff0000 time in ticks (s/96, same as song. overflows a bit over 11 minutes)
 *   0x0000f000 channel 0..4
 *   0x00000f00 waveid 0..7
 *   0x000000ff noteid 0..128
 */

struct fakesheet {

  // Constant, owner should set:
  const uint8_t *eventv;
  uint16_t eventc; // in bytes, ie event count times 4 (the same way it's stored)
  void (*cb_event)(uint32_t time_frames,uint8_t channel,uint8_t wave,uint8_t note);
  
  // Transient state managed by fakesheet:
  uint16_t eventp;
  uint32_t time;
};

/* Retains constants and resets time to zero.
 */
void fakesheet_reset(struct fakesheet *fakesheet);

/* Advance the fakesheet's clock from wherever it is to (time_frames).
 * Triggers (cb_event) for any events in that range.
 */
void fakesheet_advance(struct fakesheet *fakesheet,uint32_t time_frames);

#endif
