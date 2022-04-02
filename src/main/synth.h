/* synth.h
 */
 
#ifndef SYNTH_H
#define SYNTH_H

#include <stdint.h>

#define SYNTH_VOICE_LIMIT 8

#define SYNTH_P_SHIFT (32-9)

struct synth {
  struct synth_voice {
    const int16_t *v;
    uint32_t p;
    uint32_t pd;
    uint32_t ttl;
  } voicev[SYNTH_VOICE_LIMIT];
  uint8_t voicec;
};

int16_t synth_update(struct synth *synth);

struct synth_voice *synth_begin_note(struct synth *synth,const int16_t *wave,uint8_t noteid);
void synth_end_note(struct synth *synth,struct synth_voice *voice);

#endif
