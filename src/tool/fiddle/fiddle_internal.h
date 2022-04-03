/* fiddle_internal.h
 * Fiddle is a quick-and-dirty tool for manipulating our MIDI and Wave files.
 * While running, you can connect your sequencer to it for music playback.
 * Edit Wave files while running, and we'll pick up the change.
 */

#ifndef FIDDLE_INTERNAL_H
#define FIDDLE_INTERNAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/poll.h>
#include "tool/common/midi.h"
#include "main/synth.h"

#if PO_USE_alsa
  #include "opt/alsa/alsa.h"
#else
  #warning "No native audio driver. fiddle will not be useful."
#endif

#if PO_USE_inotify
  #include "opt/inotify/inotify.h"
#else
  #warning "inotify not enabled. Won't be able to detect changes to wave files."
#endif

#if PO_USE_inotify && PO_USE_ossmidi
  #include "opt/ossmidi/ossmidi.h"
#else
  #warning "One or both of (inotify,ossmidi) was not enabled. MIDI input not available."
#endif

extern struct fiddle {
  #if PO_USE_alsa
    struct alsa *alsa;
  #endif
  #if PO_USE_inotify
    struct inotify *inotify;
  #endif
  int rate,chanc; // populated during fiddle_drivers_init()
  int pcmlock;
  struct midi_stream midi_stream;
  struct synth synth;
  uint8_t wave_by_channel[16];
  int16_t *wavev[8]; // same as what's in (synth.wavev) but guaranteed mutable
} fiddle;

void fiddle_drivers_quit();
int fiddle_drivers_init();
int fiddle_drivers_update();
int fiddle_pcm_lock(); // driver unlocks it for you

int fiddle_cb_pcm(int16_t *v,int c,void *donttouch);
int fiddle_cb_midi(const void *src,int srcc,void *donttouch);
int fiddle_cb_inotify(const char *path,const char *base,int wd,void *userdata);

int fiddle_make_default_waves();
int fiddle_wave_possible_change(const char *path,const char *base);

#endif
