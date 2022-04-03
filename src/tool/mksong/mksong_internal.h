#ifndef MKSONG_INTERNAL_H
#define MKSONG_INTERNAL_H

#include "tool/common/tool_utils.h"
#include "tool/common/midi.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

/* midi_file_reader requires the output rate in hz.
 * We pick a number that will be easy to work with: The output tick rate times 256.
 * It's tempting to use just 96 here, but I think that would lead to rounding errors.
 */
#define MKSONG_FRAMES_PER_TICK 256
#define MIDI_READ_RATE (96*MKSONG_FRAMES_PER_TICK)

struct mksong {
  struct tool hdr;
  struct midi_file_reader *reader;
  struct encoder song;
  struct encoder fakesheet;
  struct encoder bin;
  int time; // frames
  int parttime; // time%MKSONG_FRAMES_PER_TICK; accumulates excess to smooth out timing.
  int timeticks; // ticks actually emitted (probably redundant)
  
  // Both derived from Program ID:
  uint8_t input_by_channel[16];
  uint8_t wave_by_channel[16];
  
  // Workaround for the Logic bug.
  int termsongp,termtime,termevc;
};

#define TOOL ((struct tool*)mksong)

#endif
