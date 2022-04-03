#include "platform.h"
#include "synth.h"
#include "fakesheet.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Globals.
 */

extern struct image bits;
extern struct image dancer;
extern const int16_t wave0[];
extern const int16_t wave1[];
extern const int16_t wave2[];
extern const int16_t wave3[];
extern const int16_t wave4[];
extern const int16_t wave5[];
extern const int16_t wave6[];
extern const int16_t wave7[];
extern const uint8_t inversegamma[];
extern const uint8_t carribean[];
extern const uint8_t emfeelings[];
extern const uint8_t goashuffle[];
extern const uint8_t goatpotion[];
extern const uint8_t mousechief[];
extern const uint8_t tinylamb[];

static uint16_t fbstorage[96*64];
static struct image fb={
  .v=fbstorage,
  .w=96,
  .h=64,
};

static uint8_t input=0;
static uint8_t pvinput=0;
static uint32_t framec=0;

static struct synth synth={0};
static struct fakesheet fakesheet={0};
static uint32_t lastsongtime=0;
static uint32_t song_ticks_per_beat=0; // from binary
static uint32_t song_frames_per_beat=0; // calculated here

static uint32_t totalscore=0; // bottom line. can go both up and down
static uint32_t combolength=0; // how many notes hit without a miss or overlook
static uint8_t combopower=0; // minimum score of any note in the running combo
static uint32_t totalhitc=0; // hitc+overlookc is the count of notes in the fakesheet
static uint32_t totalmissc=0; // missc is unconstrained; how many unexpected notes played by user
static uint32_t totaloverlookc=0;

/* Synthesizer.
 */

int16_t audio_next() {
  return synth_update(&synth);
}

/* Scoring.
 */
 
#define MISS_VALUE 3
#define OVERLOOK_VALUE 4
 
static void score_hit(uint8_t col,uint8_t points) {
  // Played a note acceptably close to an expected one. (points) in 0..10
  //fprintf(stderr,"%s %d +%d\n",__func__,col,points);
  
  if (!combolength) {
    combolength=1;
    combopower=points;
  } else {
    combolength++;
    if (points<combopower) combopower++;
  }
  
  totalhitc++;
  totalscore+=points;
}

static void score_miss(uint8_t col) {
  // Played a note but nothing expected on that channel.
  //fprintf(stderr,"%s %d\n",__func__,col);
  
  if (combolength) {
    //fprintf(stderr,"End of %d-note combo, power=%d\n",combolength,combopower);
    combolength=0;
  }
  
  totalmissc++;
  if (totalscore<MISS_VALUE) totalscore=0;
  else totalscore-=MISS_VALUE;
}

static void score_overlook(uint8_t col) {
  // Note slipped past the user unplayed.
  //fprintf(stderr,"%s %d\n",__func__,col);
  
  if (combolength) {
    //fprintf(stderr,"End of %d-note combo, power=%d\n",combolength,combopower);
    combolength=0;
  }
  
  totaloverlookc++;
  if (totalscore<OVERLOOK_VALUE) totalscore=0;
  else totalscore-=OVERLOOK_VALUE;
}

/* Notes XXX TEMP get this organized
 */
 
#define NOTEC 16

#define PEEK_TIME_FRAMES (22050*1)
#define DROP_TIME_FRAMES 11025
 
static struct note {
  uint8_t col; // 0..4: both horz position and icon
  int8_t y; // 0..63 (or maybe a bit further); vertical midpoint
  uint8_t scored;
  uint32_t time; // absolute song time in frames
  uint8_t waveid;
  uint8_t noteid;
} notev[NOTEC]={0};

static void init_notes() {
  struct note *note=notev;
  uint8_t i=NOTEC;
  for (;i-->0;note++) {
    note->col=0xff;
  }
}

static void drop_all_notes() {
  struct note *note=notev;
  uint8_t i=NOTEC;
  for (;i-->0;note++) {
    note->col=0xff;
  }
}

/* Receive new upcoming event from fakesheet.
 */
 
static void cb_fakesheet_event(uint32_t time,uint8_t channel,uint8_t waveid,uint8_t noteid) {
  struct note *note=notev;
  uint8_t i=NOTEC;
  for (;i-->0;note++) {
    if (note->col<5) continue;
    note->col=channel;
    note->waveid=waveid;
    note->noteid=noteid;
    note->time=time;
    note->scored=0;
    return;
  }
}

/* Replace (y) in each live note, based on the given absolute song time in frames.
 * If a note is aged out, this drops it and scores the miss.
 */
 
static void reposition_notes(int32_t now) {
  struct note *note=notev;
  uint8_t i=NOTEC;
  for (;i-->0;note++) {
    if (note->col>=5) continue;
    int32_t reltime=note->time-now;
    
    if (reltime<-DROP_TIME_FRAMES) {
      if (!note->scored) {
        score_overlook(note->col);
      }
      note->col=5;
      continue;
    }
    
    if (reltime>=PEEK_TIME_FRAMES) {
      note->y=-10;
      continue;
    }
    
    note->y=52-(reltime*62)/PEEK_TIME_FRAMES;
  }
}

/* Consider the song's current time.
 * Create new notes if warranted, and reposition all the ones we have.
 */

static void update_notes() {

  if (!synth.song) {
    drop_all_notes();
    return;
  }
  
  // Advance fakesheet to the song's time, plus our view window length. May create new notes.
  if (synth.songtime<lastsongtime) {
    fakesheet_reset(&fakesheet);
    drop_all_notes();
  }
  lastsongtime=synth.songtime;
  if (synth.songhold) {
    if (synth.songhold<PEEK_TIME_FRAMES) {
      fakesheet_advance(&fakesheet,PEEK_TIME_FRAMES-synth.songhold);
    }
  } else {
    fakesheet_advance(&fakesheet,synth.songtime+PEEK_TIME_FRAMES);
  }
  
  if (synth.songhold<PEEK_TIME_FRAMES) {
    reposition_notes(synth.songtime-synth.songhold);
  }
}

/* A key was pressed.
 * Find the nearest unscored note in that column and score it, or register a botch.
 */
 
#define DISTANCE_LIMIT 10 /* pixels */

static void play_note(uint8_t col) {
  struct note *best=0;
  struct note *note=notev;
  uint8_t bestdistance=DISTANCE_LIMIT;
  uint8_t i=NOTEC;
  for (;i-->0;note++) {
    if (note->scored) continue;
    if (note->col!=col) continue;
    int8_t distance=note->y-52; // The "now" line is at y==52
    if (distance<0) distance=-distance;
    if (distance>bestdistance) continue;
    best=note;
    bestdistance=distance;
  }
  
  //TODO fireworks or something
  
  if (best) {
    int8_t score=DISTANCE_LIMIT-bestdistance;
    synth_note_fireforget(&synth,best->waveid,best->noteid,0x10);
    best->scored=1;
    score_hit(col,score);
  } else {
    synth_note_fireforget(&synth,0,0x30,0x10);
    synth_note_fireforget(&synth,0,0x36,0x10);
    score_miss(col);
  }
}

/* Render decimal integer.TODO replace color
 */
 
static void render_int(struct image *dst,int16_t dstx,int16_t dsty,int32_t n,uint8_t digitc) {

  // Sign if negative only, and it does count toward (digitc).
  if (n<0) {
    image_blit_colorkey(dst,dstx,dsty,&bits,40,71,4,7);
    dstx+=4;
    digitc--;
    n=-n; // i hope it's not INT_MIN
  }
  
  uint8_t digiti=digitc;
  for (;digiti-->0;n/=10) {
    uint8_t digit=n%10;
    image_blit_colorkey(dst,dstx+digiti*5,dsty,&bits,digit*4,71,4,7);
  }
}

/* Render frame.
 */
 
static void render(uint32_t beatc) {
  
  // Black out.
  memset(fbstorage,0,sizeof(fbstorage));
  
  // 5 track backgrounds.
  image_blit_opaque(&fb, 8,0,&bits,8,7,2,64);
  image_blit_opaque(&fb,20,0,&bits,8,7,2,64);
  image_blit_opaque(&fb,32,0,&bits,8,7,2,64);
  image_blit_opaque(&fb,44,0,&bits,8,7,2,64);
  image_blit_opaque(&fb,56,0,&bits,8,7,2,64);
  
  // Notes.
  {
    struct note *note=notev;
    uint8_t i=NOTEC;
    for (;i-->0;note++) {
      if (note->col>4) continue; // Can use this as "none" indicator.
      int16_t srcy=note->scored?31:22;
      image_blit_colorkey(&fb,4+note->col*12,note->y-4,&bits,10+note->col*10,srcy,10,9);
    }
  }
  
  // "Now" line.
  image_blit_colorkey(&fb,0,52,&bits,11,21,66,1);
  
  // 5 button indicators.
  #define BTN(ix,tag) image_blit_colorkey(&fb,3+ix*12,56,&bits,10+ix*12,7+((input&BUTTON_##tag)?7:0),12,7);
  BTN(0,LEFT)
  BTN(1,UP)
  BTN(2,RIGHT)
  BTN(3,B)
  BTN(4,A)
  #undef BTN
  
  // Score.
  render_int(&fb,69,3,totalscore,5);
  
  // Dancing bear. TODO
  image_blit_colorkey(&fb,75,12,&dancer,(beatc&3)*12,0,12,24);
  
  // Combo quality indicator.
  image_blit_opaque(&fb,69,37,&bits,10,40,24,24);
  int16_t combow=combolength*2;
  if (combow>24) combow=24;
  image_blit_opaque(&fb,69,37,&bits,34,40,combow,24);
  if (combolength>=15) { // "Captain! Sensors indicate that music is happening!"
    if (!(beatc&1)) {
      image_blit_colorkey(&fb,69,37,&bits,58,40,24,24);
    }
  }
  
  // Frames. (note that the corners overlap, that's ok)
  image_blit_colorkey(&fb,0,0,&bits,4,7,4,64); // left
  image_blit_colorkey(&fb,92,0,&bits,0,7,4,64); // right
  image_blit_colorkey(&fb,0,0,&bits,0,3,96,4); // top
  image_blit_colorkey(&fb,0,60,&bits,0,0,96,4); // bottom
  image_blit_colorkey(&fb,62,0,&bits,0,7,8,64); // full-height horz divider
}

/* Main loop.
 */
 
void loop() {
  framec++;

  input=platform_update();
  if (input!=pvinput) {
    #define PRESS(tag) ((input&BUTTON_##tag)&&!(pvinput&BUTTON_##tag))
         if (PRESS(LEFT)) play_note(0);
    else if (PRESS(UP)) play_note(1);
    else if (PRESS(RIGHT)) play_note(2);
    else if (PRESS(B)) play_note(3);
    else if (PRESS(A)) play_note(4);
    #undef DOWN
    pvinput=input;
  }
  
  update_notes();
  
  uint32_t beatc=synth.songtime/song_frames_per_beat;
  
  render(beatc);
  platform_send_framebuffer(fb.v);
}

/* Unpack encoded song and fakesheet.
 */

static void load_song(const uint8_t *src) {

  // We don't record length of (src). I guess that's ok, since it's generated at build time and therefore trusted.
  const uint16_t *hdr=(uint16_t*)src;
  song_frames_per_beat=hdr[0];
  if (!song_frames_per_beat) {
    fprintf(stderr,"%s:%d: frames/beat zero! Defaulting to 11025\n",__FILE__,__LINE__);
  } else {
    fprintf(stderr,"%s:%d song_frames_per_beat=%d\n",__FILE__,__LINE__,song_frames_per_beat);
  }
  const uint16_t hdrlen=8;
  uint16_t addlhdrlen=hdr[1];
  uint16_t songlen=hdr[2];
  uint16_t fakesheetlen=hdr[3];
  
  synth.song=src+hdrlen+addlhdrlen;
  synth.songc=songlen;
  synth.songhold=22050;
  
  fakesheet.cb_event=cb_fakesheet_event;
  fakesheet.eventv=(uint32_t*)(src+hdrlen+addlhdrlen+songlen);
  fakesheet.eventc=fakesheetlen>>2;
}

/* Init.
 */

void setup() {
  platform_init();
  
  synth.wavev[0]=wave0;
  synth.wavev[1]=wave1;
  synth.wavev[2]=wave2;
  synth.wavev[3]=wave3;
  synth.wavev[4]=wave4;
  synth.wavev[5]=wave5;
  synth.wavev[6]=wave6;
  synth.wavev[7]=wave7;
  
  /*
src/data/embed/carribean.mid TODO 0:45 head/tail,instruments,input -- sitter
src/data/embed/emfeelings.mid TODO 0:50 head/tail,instruments,input -- sitter
src/data/embed/goashuffle.mid TODO 0:43 head/tail,instruments,input -- sitter
src/data/embed/goatpotion.mid TODO 1:10 instruments,input -- original
src/data/embed/inversegamma.mid TODO 1:41 instruments,input -- original
src/data/embed/mousechief.mid TODO 1:01 head(tail ok?),instruments,input -- sitter
src/data/embed/tinylamb.mid 0:35 instruments,input -- original
  */
  load_song(goatpotion);
  
  init_notes();
}
