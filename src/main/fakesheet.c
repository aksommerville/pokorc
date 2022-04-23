#include "fakesheet.h"
#include "synth.h"

/* Reset.
 */
 
void fakesheet_reset(struct fakesheet *fakesheet) {
  fakesheet->eventp=0;
  fakesheet->time=0;
}

/* Advance time and trigger callback.
 */
 
void fakesheet_advance(struct fakesheet *fakesheet,uint32_t time_frames) {
  if (time_frames<=fakesheet->time) return;
  while (fakesheet->eventp<=fakesheet->eventc-4) {
    // This used to be uint32_t, and it worked most of the time.
    // But I guess depending on some compile-time voodoo, sometimes Arduino won't read uint32_t from a PROGMEM uint8_t[].
    const uint8_t *b=fakesheet->eventv+fakesheet->eventp;
    uint32_t event=(b[0]|(b[1]<<8)|(b[2]<<16)|(b[3]<<24));
    uint32_t etime=event>>16;
    etime*=fakesheet->frames_per_tick;
    if (etime>time_frames) break;
    fakesheet->eventp+=4;
    uint8_t channel=(event>>12)&0x0f;
    uint8_t waveid=(event>>8)&0x0f;
    uint8_t noteid=event&0xff;
    fakesheet->cb_event(etime,channel,waveid,noteid);
  }
  fakesheet->time=time_frames;
}
