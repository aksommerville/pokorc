#include "game.h"
#include "platform.h"
#include "data.h"
#include "synth.h"
#include "fakesheet.h"
#include <string.h>
#include <stdio.h>

/* Globals.
 */

static uint32_t totalscore=0; // bottom line. can go both up and down
static uint32_t combolength=0; // how many notes hit without a miss or overlook
static uint8_t combopower=0; // minimum score of any note in the running combo
static uint32_t totalhitc=0; // hitc+overlookc is the count of notes in the fakesheet
static uint32_t totalmissc=0; // missc is unconstrained; how many unexpected notes played by user
static uint32_t totaloverlookc=0;

static uint32_t lastsongtime=0;
static uint32_t beatc=0;
static uint32_t song_ticks_per_beat=0; // from binary
static uint32_t song_frames_per_beat=11025; // calculated here

static struct {
  uint8_t waveid;
  uint8_t noteid;
} notes_by_input[5]={0};

#define TOAST_LIMIT 16
static struct toast {
  uint8_t type;
  uint8_t col;
  int8_t score;
  uint8_t ttl;
} toastv[TOAST_LIMIT];
#define TOAST_TYPE_NONE 0
#define TOAST_TYPE_POINTS 1
#define TOAST_TYPE_X 2

#define NOTEC 16
static struct note {
  uint8_t col; // 0..4: both horz position and icon
  int8_t y; // 0..63 (or maybe a bit further); vertical midpoint
  uint8_t scored;
  uint32_t time; // absolute song time in frames
  uint8_t waveid;
  uint8_t noteid;
} notev[NOTEC]={0};

static struct synth *synth=0;
static struct fakesheet *fakesheet=0;
static uint8_t input=0;

/* Scoring.
 */
 
#define MISS_VALUE 3
#define OVERLOOK_VALUE 4
 
static uint8_t score_hit(uint8_t col,uint8_t points) {
  // Played a note acceptably close to an expected one. (points) in 0..10
  //fprintf(stderr,"%s %d +%d\n",__func__,col,points);
  
  if (!combolength) {
    combolength=1;
    combopower=points;
  } else {
    combolength++;
    if (points<combopower) combopower++;
  }
  
  if (combolength>=20) points*=3;
  else if (combolength>=15) points*=2;
  
  totalhitc++;
  totalscore+=points;
  
  return points;
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

/* Notes
 */
 
#define PEEK_TIME_FRAMES (22050*1)
#define DROP_TIME_FRAMES 11025

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

/* Begin.
 */
 
void game_begin(
  const struct songinfo *songinfo,
  struct synth *_synth,
  struct fakesheet *_fakesheet
) {
  synth=_synth;
  fakesheet=_fakesheet;

  // We don't record length of (src). I guess that's ok, since it's generated at build time and therefore trusted.
  const uint8_t *hdr=songinfo->song;
  song_frames_per_beat=hdr[0]|(hdr[1]<<8);
  if (!song_frames_per_beat) {
    fprintf(stderr,"%s:%d:%s: frames/beat zero! Defaulting to 11025\n",__FILE__,__LINE__,songinfo->name);
    song_frames_per_beat=11025;
  } else {
    fprintf(stderr,"%s:%d:%s: song_frames_per_beat=%d\n",__FILE__,__LINE__,songinfo->name,song_frames_per_beat);
  }
  beatc=0;
  const uint16_t hdrlen=8;
  uint16_t addlhdrlen=hdr[2]|(hdr[3]<<8);
  uint16_t songlen=hdr[4]|(hdr[5]<<8);
  uint16_t fakesheetlen=hdr[6]|(hdr[7]<<8);
  
  synth->song=songinfo->song+hdrlen+addlhdrlen;
  synth->songc=songlen;
  synth->songhold=22050;
  
  fakesheet->cb_event=cb_fakesheet_event;
  fakesheet->eventv=songinfo->song+hdrlen+addlhdrlen+songlen;
  fakesheet->eventc=fakesheetlen;
  
  init_notes();
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

  if (!synth->song) {
    drop_all_notes();
    return;
  }
  
  // Advance fakesheet to the song's time, plus our view window length. May create new notes.
  if (synth->songtime<lastsongtime) {
    fakesheet_reset(fakesheet);
    drop_all_notes();
  }
  lastsongtime=synth->songtime;
  if (synth->songhold) {
    if (synth->songhold<PEEK_TIME_FRAMES) {
      fakesheet_advance(fakesheet,PEEK_TIME_FRAMES-synth->songhold);
    }
  } else {
    fakesheet_advance(fakesheet,synth->songtime+PEEK_TIME_FRAMES);
  }
  
  if (synth->songhold<PEEK_TIME_FRAMES) {
    reposition_notes(synth->songtime-synth->songhold);
  }
}

/* Add toasts attached to one input channel.
 */
 
static void add_score_toast(uint8_t col,uint8_t points) {
  struct toast *toast=toastv;
  uint8_t i=TOAST_LIMIT;
  for (;i-->0;toast++) {
    if (toast->type) continue;
    toast->type=TOAST_TYPE_POINTS;
    toast->col=col;
    toast->score=points;
    toast->ttl=30;
    return;
  }
}

static void add_miss_toast(uint8_t col) {
  struct toast *toast=toastv;
  uint8_t i=TOAST_LIMIT;
  for (;i-->0;toast++) {
    if (toast->type) continue;
    toast->type=TOAST_TYPE_X;
    toast->col=col;
    toast->score=0;
    toast->ttl=30;
    return;
  }
}

static void update_toasts() {
  struct toast *toast=toastv;
  uint8_t i=TOAST_LIMIT;
  for (;i-->0;toast++) {
    if (!toast->type) continue;
    if (toast->ttl) toast->ttl--;
    else toast->type=0;
  }
}

/* User notes on and off.
 * Find the nearest unscored note in that column and score it, or register a botch.
 */
 
#define DISTANCE_LIMIT 10 /* pixels */

static void play_note(uint8_t col) {
  if (col>=5) return;
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
  
  if (best) {
    int8_t score=DISTANCE_LIMIT-bestdistance;
    notes_by_input[col].waveid=best->waveid;
    notes_by_input[col].noteid=best->noteid;
    synth_note_on(synth,best->waveid,best->noteid);
    best->scored=1;
    uint8_t points=score_hit(col,score);
    add_score_toast(col,points);
  } else {
    synth_note_fireforget(synth,0,0x30,0x10);
    synth_note_fireforget(synth,0,0x36,0x10);
    score_miss(col);
    add_miss_toast(col);
  }
}

static void drop_note(uint8_t col) {
  if (col>=5) return;
  if (notes_by_input[col].noteid) {
    synth_note_off(synth,notes_by_input[col].waveid,notes_by_input[col].noteid);
    notes_by_input[col].waveid=0;
    notes_by_input[col].noteid=0;
  }
}

/* Receive input.
 */
 
void game_input(uint8_t _input,uint8_t pvinput) {
  input=_input;
  #define BTN(tag,col) \
    if ((input&BUTTON_##tag)&&!(pvinput&BUTTON_##tag)) play_note(col); \
    else if (!(input&BUTTON_##tag)&&(pvinput&BUTTON_##tag)) drop_note(col);
  BTN(LEFT,0)
  BTN(UP,1)
  BTN(RIGHT,2)
  BTN(B,3)
  BTN(A,4)
  #undef BTN
}

/* Render decimal integer.TODO replace color
 */
 
static void render_int(struct image *dst,int16_t dstx,int16_t dsty,int32_t n,uint8_t digitc) {
  if (!digitc) {
    if (n>=10000) digitc=5;
    else if (n>=1000) digitc=4;
    else if (n>=100) digitc=3;
    else if (n>=10) digitc=2;
    else if (n>=0) digitc=1;
    else if (n>-10) digitc=2;
    else if (n>-100) digitc=3;
    else if (n>-1000) digitc=4;
    else digitc=5;
  }

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

/* Render.
 */
 
void game_render(struct image *fb) {
  
  // Black out.
  memset(fb->v,0,fb->w*fb->h*2);
  
  // 5 track backgrounds.
  image_blit_opaque(fb, 8,0,&bits,8,7,2,64);
  image_blit_opaque(fb,20,0,&bits,8,7,2,64);
  image_blit_opaque(fb,32,0,&bits,8,7,2,64);
  image_blit_opaque(fb,44,0,&bits,8,7,2,64);
  image_blit_opaque(fb,56,0,&bits,8,7,2,64);
  
  // Notes.
  {
    struct note *note=notev;
    uint8_t i=NOTEC;
    for (;i-->0;note++) {
      if (note->col>4) continue; // Can use this as "none" indicator.
      int16_t srcy=note->scored?31:22;
      image_blit_colorkey(fb,4+note->col*12,note->y-4,&bits,10+note->col*10,srcy,10,9);
    }
  }
  
  // "Now" line.
  image_blit_colorkey(fb,0,52,&bits,11,21,66,1);
  
  // 5 button indicators.
  #define BTN(ix,tag) image_blit_colorkey(fb,3+ix*12,56,&bits,10+ix*12,7+((input&BUTTON_##tag)?7:0),12,7);
  BTN(0,LEFT)
  BTN(1,UP)
  BTN(2,RIGHT)
  BTN(3,B)
  BTN(4,A)
  #undef BTN
  
  // Fireworks.
  {
    struct toast *toast=toastv;
    uint8_t i=TOAST_LIMIT;
    for (;i-->0;toast++) switch (toast->type) {
      case TOAST_TYPE_POINTS: {
          render_int(fb,3+toast->col*12,30+(toast->ttl>>1),toast->score,0);
        } break;
      case TOAST_TYPE_X: {
          image_blit_colorkey(fb,3+toast->col*12,30+(toast->ttl>>1),&bits,0,85,11,11);
        } break;
    }
  }
  
  // Score.
  render_int(fb,69,3,totalscore,5);
  
  // Dancer. TODO fancier
  uint32_t subtiming=synth->songtime%song_frames_per_beat;
  uint8_t frame=(subtiming*4)/song_frames_per_beat;
  int16_t srcx;
  switch (frame) {
    case 0: srcx=12*0; break;
    case 1: srcx=12*4; break;
    case 2: srcx=12*0; break;
    case 3: srcx=12*5; break;
  }
  image_blit_colorkey(fb,75,12,&dancer,srcx,0,12,24);
  
  // Combo quality indicator.
  image_blit_opaque(fb,69,37,&bits,10,40,24,24);
  int16_t combow=combolength*2;
  if (combow>24) combow=24;
  image_blit_opaque(fb,69,37,&bits,34,40,combow,24);
  if (combolength>=15) { // "Captain! Sensors indicate that music is happening!"
    if (!(beatc&1)) {
      image_blit_colorkey(fb,69,37,&bits,58,40,24,24);
    }
  }
  
  // Frames. (note that the corners overlap, that's ok)
  image_blit_colorkey(fb,0,0,&bits,4,7,4,64); // left
  image_blit_colorkey(fb,92,0,&bits,0,7,4,64); // right
  image_blit_colorkey(fb,0,0,&bits,0,3,96,4); // top
  image_blit_colorkey(fb,0,60,&bits,0,0,96,4); // bottom
  image_blit_colorkey(fb,62,0,&bits,0,7,8,64); // full-height horz divider
}

/* Update.
 */
 
void game_update() {
  beatc=synth->songtime/song_frames_per_beat;
  update_notes();
  update_toasts();
}
