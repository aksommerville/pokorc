#include "fiddle_internal.h"
#include <signal.h>

struct fiddle fiddle={0};

/* Signal handler.
 */
 
static volatile int sigc=0;

static void rcvsig(int sigid) {
  switch (sigid) {
    case SIGINT: if (++sigc>=3) {
        fprintf(stderr,"Too many unprocessed signals.\n");
        exit(1);
      } break;
  }
}

/* PCM callback.
 */
 
int fiddle_cb_pcm(int16_t *v,int c,void *donttouch) {
  switch (fiddle.chanc) {
    case 1: {
        for (;c-->0;v++) *v=synth_update(&fiddle.synth);
      } break;
    case 2: {
        c>>=1;
        for (;c-->0;v+=2) v[0]=v[1]=synth_update(&fiddle.synth);
      } break;
    default: {
        memset(v,0,c<<1);
      }
  }
  return 0;
}

/* Process one MIDI event.
 */
 
static int fiddle_midi_event(const struct midi_event *event) {
  if (fiddle_pcm_lock()<0) return -1;
  switch (event->opcode) {
    case MIDI_OPCODE_PROGRAM: {
        fiddle.wave_by_channel[event->chid&0xf]=event->a&7;
        synth_release_all(&fiddle.synth); // heavy-handed, sorry
      } return 0;
    case MIDI_OPCODE_NOTE_ON: synth_note_on(&fiddle.synth,fiddle.wave_by_channel[event->chid&0xf],event->a); return 0;
    case MIDI_OPCODE_NOTE_OFF: synth_note_off(&fiddle.synth,fiddle.wave_by_channel[event->chid&0xf],event->a); return 0;
    case MIDI_OPCODE_SYSTEM_RESET: synth_silence_all(&fiddle.synth); return 0;
    case MIDI_OPCODE_CONTROL: switch (event->a) {
        case MIDI_CONTROL_SOUND_OFF: synth_silence_all(&fiddle.synth); return 0;
        case MIDI_CONTROL_NOTES_OFF: synth_release_all(&fiddle.synth); return 0;
      } break;
  }
  return 0;
}

/* MIDI callback.
 */
 
int fiddle_cb_midi(const void *src,int srcc,void *donttouch) {
  const uint8_t *SRC=src;
  /**
  fprintf(stderr,"%s:%d:",__func__,srcc);
  int i=0; for (;i<srcc;i++) fprintf(stderr," %02x",SRC[i]);
  fprintf(stderr,"\n");
  /**/
  int srcp=0;
  while (srcp<srcc) {
    struct midi_event event={0};
    int err=midi_stream_decode(&event,&fiddle.midi_stream,SRC+srcp,srcc-srcp);
    if (err<=0) {
      fprintf(stderr,"Error parsing MIDI in. Dropping %d bytes.\n",srcc-srcp);
      break;
    }
    //fprintf(stderr,"  %02x %02x %02x %02x %d\n",event.opcode,event.chid,event.a,event.b,event.c);
    srcp+=err;
    if (fiddle_midi_event(&event)<0) return -1;
  }
  return 0;
}

/* Inotify callback, check for wave files.
 */
 
int fiddle_cb_inotify(const char *path,const char *base,int wd,void *userdata) {
  //fprintf(stderr,"%s %s\n",__func__,path);
  int err=fiddle_wave_possible_change(path,base);
  if (err) return err;
  return 0;
}

/* Main.
 */
 
int main(int argc,char **argv) {
  signal(SIGINT,rcvsig);
  if (fiddle_make_default_waves()<0) return -1;
  if (fiddle_drivers_init()<0) return -1;
  
  fprintf(stderr,"%s: Running. SIGINT to quit...\n",argv[0]);
  while (!sigc) {
    usleep(1000);
    if (fiddle_drivers_update()<0) return -1;
  }
  
  fiddle_drivers_quit();
  fprintf(stderr,"%s: Normal exit.\n",argv[0]);
  return 0;
}
