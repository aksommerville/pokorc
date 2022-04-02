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
  while (fakesheet->eventp<fakesheet->eventc) {
    uint32_t event=fakesheet->eventv[fakesheet->eventp];
    uint32_t etime=event>>16;
    etime*=SYNTH_FRAMES_PER_TICK;
    if (etime>time_frames) break;
    fakesheet->eventp++;
    uint8_t channel=(event>>12)&0x0f;
    uint8_t waveid=(event>>8)&0x0f;
    uint8_t noteid=event&0xff;
    fakesheet->cb_event(etime,channel,waveid,noteid);
  }
  fakesheet->time=time_frames;
}
