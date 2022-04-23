/* synth.h
 */
 
#ifndef SYNTH_H
#define SYNTH_H

#include <stdint.h>

#define SYNTH_VOICE_LIMIT 8
#define SYNTH_WAVE_COUNT 8
#define SYNTH_TICKS_PER_SECOND 96 /* approximately */

#define SYNTH_P_SHIFT (32-9)

struct synth {
  struct synth_voice {
    const int16_t *v;
    uint32_t p;
    uint32_t pd;
    uint32_t ttl;
    uint8_t waveid,noteid; // for identification
  } voicev[SYNTH_VOICE_LIMIT];
  uint8_t voicec;
  
  // Owner should populate directly.
  const int16_t *wavev[SYNTH_WAVE_COUNT];
  uint32_t songhold; // extra delay before starting song, frames.
  
  const uint8_t *song;
  uint16_t songc;
  uint16_t songp;
  uint32_t songdelay;
  uint32_t songtime; // frames since start
  
  int32_t frames_per_tick;
  int32_t rate;
  int32_t release_time;
};

/* Beware, if you initialize more than once with different rates,
 * rounding error will accumulate and gradually detune the notes.
 */
int8_t synth_init(struct synth *synth,int32_t rate);

int16_t synth_update(struct synth *synth);

/* Loose note commands, caller supplies a 512-sample wave.
 */
struct synth_voice *synth_begin_note(struct synth *synth,const int16_t *wave,uint8_t noteid);
void synth_end_note(struct synth *synth,struct synth_voice *voice);
struct synth_voice *synth_fireforget_note(struct synth *synth,const int16_t *wave,uint8_t noteid,uint32_t durframes);

/* Note commands matching our serial song format.
 */
void synth_note_fireforget(struct synth *synth,uint8_t waveid,uint8_t noteid,uint8_t durticks);
void synth_note_on(struct synth *synth,uint8_t waveid,uint8_t noteid);
void synth_note_off(struct synth *synth,uint8_t waveid,uint8_t noteid);

void synth_release_all(struct synth *synth);
void synth_silence_all(struct synth *synth);

#endif
