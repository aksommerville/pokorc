#include "genioc_internal.h"
#include <signal.h>

struct genioc genioc={0};

/* argv
 */
 
static void genioc_print_help(const char *exename) {
  fprintf(stderr,"Usage: %s [OPTIONS]\n",exename);
  fprintf(stderr,
    "OPTIONS:\n"
    "  --fullscreen           GLX only, start fullscreen.\n"
    "  --video-device=PATH    DRM only.\n"
    "  --video-rate=HZ        DRM only, guides our mode selection, not exact.\n"
    "  --glsl-version=INT     DRM only, default 100.\n"
    "  --audio-device=PATH    ALSA only.\n"
    "  --audio-rate=INT       Default 22050.\n"
    "  --audio-chanc=INT      Default 1. In stereo, we output the same thing L and R.\n"
  );
}

static int genioc_argv_get_boolean(int argc,char **argv,const char *k) {
  int p=1; for (;p<argc;p++) {
    if (!strcmp(argv[p],k)) return 1;
  }
  return 0;
}

static int genioc_argv_get_int(int argc,char **argv,const char *k,int fallback) {
  int kc=0; while (k[kc]) kc++;
  int p=1; for (;p<argc;p++) {
    if (memcmp(argv[p],k,kc)) continue;
    if (argv[p][kc]!='=') continue;
    int v=0;
    const char *src=argv[p]+kc+1;
    for (;*src;src++) {
      if ((*src<'0')||(*src>'9')) return fallback;
      v*=10;
      v+=(*src)-'0';
    }
    return v;
  }
  return fallback;
}

static const char *genioc_argv_get_string(int argc,char **argv,const char *k,const char *fallback) {
  int kc=0; while (k[kc]) kc++;
  int p=1; for (;p<argc;p++) {
    if (memcmp(argv[p],k,kc)) continue;
    if (argv[p][kc]!='=') continue;
    return argv[p]+kc+1;
  }
  return fallback;
}

/* Quit drivers.
 */
 
static void genioc_quit_drivers() {
  #if PO_USE_x11
    po_x11_del(genioc.x11);
  #endif
  #if PO_USE_drmfb
    drmfb_del(genioc.drmfb);
  #endif
  #if PO_USE_drmgx
    drmgx_del(genioc.drmgx);
  #endif
  #if PO_USE_bcm
    bcm_del(genioc.bcm);
  #endif
  #if PO_USE_alsa
    alsa_del(genioc.alsa);
  #endif
  #if PO_USE_evdev
    po_evdev_del(genioc.evdev);
  #endif
}

/* Signal handler.
 */
 
static void genioc_rcvsig(int sigid) {
  switch (sigid) {
    case SIGINT: if (++(genioc.sigc)>=3) {
        fprintf(stderr,"Too many unprocessed signals.\n");
        exit(1);
      } break;
  }
}

/* Driver callbacks.
 */
 
#if PO_USE_x11
 
static int genioc_cb_x11_button(struct po_x11 *x11,uint8_t btnid,int value) {
  if (value) genioc.inputstate|=btnid;
  else genioc.inputstate&=~btnid;
  return 0;
}

static int genioc_cb_x11_close(struct po_x11 *x11) {
  genioc.terminate=1;
  return 0;
}

#endif

static void genioc_cb_pcm(int16_t *v,int c,int chanc,int driver_rate) {
  if (chanc<1) {
    memset(v,0,c<<1);
    return;
  }
  switch (chanc) {
  
    case 1: {
        for (;c-->0;v++) *v=audio_next();
      } break;
      
    case 2: {
        int framec=c>>1;
        for (;framec-->0;v+=2) v[0]=v[1]=audio_next();
      } break;
      
    default: {
        // (chanc>2), I reckon this is unlikely to happen so not bothering with rate correction.
        int framec=c/chanc;
        for (;framec-->0;) {
          int16_t sample=audio_next();
          int i=chanc;
          for (;i-->0;v++) *v=sample;
        }
      }
  }
}

#if PO_USE_alsa

static int genioc_cb_alsa(int16_t *v,int c,struct alsa *alsa) {
  genioc_cb_pcm(v,c,alsa_get_chanc(alsa),alsa_get_rate(alsa));
  return 0;
}

#endif

#if PO_USE_evdev

static int genioc_cb_evdev(struct po_evdev *evdev,uint8_t btnid,int value) {
  if (btnid<BUTTON_NONSTANDARD) {
    if (value) genioc.inputstate|=btnid;
    else genioc.inputstate&=~btnid;
  } else switch (btnid) {
    case BUTTON_QUIT: genioc.terminate=1; break;
  }
  #if PO_USE_x11
    po_x11_inhibit_screensaver(genioc.x11);
  #endif
  return 0;
}

#endif

/* Init video driver.
 */

static int genioc_init_video_driver(int argc,char **argv) {
  #if PO_USE_x11
    if (genioc.x11=po_x11_new(
      "Pocket Orchestra",
      96,64,genioc_argv_get_boolean(argc,argv,"--fullscreen"),
      genioc_cb_x11_button,
      genioc_cb_x11_close,
      &genioc
    )) {
      fprintf(stderr,"Using X11 for video.\n");
      return 0;
    }
  #endif
  
  #if PO_USE_drmfb
    if (genioc.drmfb=drmfb_new(96,64,genioc_argv_get_int(argc,argv,"--video-rate",60))) {
      fprintf(stderr,"Using DRM unaccelerated for video.\n");
      return 0;
    }
  #endif

  #if PO_USE_drmgx
    if (genioc.drmgx=drmgx_new(
      genioc_argv_get_string(argc,argv,"--video-device",0),
      genioc_argv_get_int(argc,argv,"--video-rate",60),
      96,64,DRMGX_FMT_TINY16,
      genioc_argv_get_int(argc,argv,"--video-filter",0),
      genioc_argv_get_int(argc,argv,"--glsl-version",0)
    )) {
      fprintf(stderr,"Using DRM with GLES for video.\n");
      return 0;
    }
  #endif

  #if PO_USE_bcm
    if (genioc.bcm=bcm_new(96,64)) {
      fprintf(stderr,"Using BCM for video.\n");
      return 0;
    }
  #endif
  
  fprintf(stderr,"Unable to initialize any video driver.\n");
  return -1;
}

/* Init audio driver.
 */
 
static int genioc_init_audio_driver(int argc,char **argv) {
  #if PO_USE_alsa
    struct alsa_delegate alsa_delegate={
      .rate=genioc_argv_get_int(argc,argv,"--audio-rate",22050),
      .chanc=genioc_argv_get_int(argc,argv,"--audio-chanc",1),
      .device=genioc_argv_get_string(argc,argv,"--audio-device",0),
      .cb_pcm_out=genioc_cb_alsa,
    };
    if (genioc.alsa=alsa_new(&alsa_delegate)) {
      fprintf(stderr,"Using ALSA for audio. rate=%d chanc=%d\n",alsa_get_rate(genioc.alsa),alsa_get_chanc(genioc.alsa));
      return 0;
    }
  #endif
  
  fprintf(stderr,"Unable to initialize any audio driver.\n");
  return -1;
}

/* Init drivers.
 */
 
static int genioc_init_drivers(int argc,char **argv) {

  signal(SIGINT,genioc_rcvsig);

  if (genioc_init_video_driver(argc,argv)<0) return -1;
  if (genioc_init_audio_driver(argc,argv)<0) return -1;
  
  #if PO_USE_evdev
    if (!(genioc.evdev=po_evdev_new(genioc_cb_evdev,&genioc))) {
      fprintf(stderr,"Failed to initialize evdev. Proceeding without joystick support.\n");
    }
  #endif
  
  return 0;
}

/* Init per client (noop).
 */
 
uint8_t platform_init(int32_t *audio_rate) {

  if (0) ;
  #if PO_USE_alsa
    else if (genioc.alsa) *audio_rate=alsa_get_rate(genioc.alsa);
  #endif
  else *audio_rate=0;
  
  return 1;
}

/* Update.
 */
 
uint8_t platform_update() {
  #if PO_USE_x11
    if (genioc.x11) {
      po_x11_update(genioc.x11);
    }
  #endif
  #if PO_USE_evdev
    if (genioc.evdev) {
      po_evdev_update(genioc.evdev);
    }
  #endif
  return genioc.inputstate;
}

/* Receive framebuffer.
 * We run the whole client update inside an audio lock.
 * There's reasons for that, don't worry about it.
 * But the video swap might block, and that could be a problem.
 * So we'll only record the framebuffer address when client sends it, and commit once the audio is unlocked.
 * This introduces a danger that changes to the framebuffer after platform_send_framebuffer() would be sent early.
 * Pretty safe to assume that the app is not doing that, that its platform_send_framebuffer() is the very end of the cycle.
 */
 
void platform_send_framebuffer(const void *fb) {
  genioc.fbtmp=fb;
}

static void genioc_finish_video_frame() {
  if (!genioc.fbtmp) return;
  #if PO_USE_x11
    if (genioc.x11) {
      po_x11_swap(genioc.x11,genioc.fbtmp);
      return;
    }
  #endif
  #if PO_USE_drmfb
    if (genioc.drmfb) {
      drmfb_swap(genioc.drmfb,genioc.fbtmp);
      return;
    }
  #endif
  #if PO_USE_drmgx
    if (genioc.drmgx) {
      drmgx_swap(genioc.drmgx,genioc.fbtmp);
      return;
    }
  #endif
  #if PO_USE_bcm
    if (genioc.bcm) {
      bcm_swap(genioc.bcm,genioc.fbtmp);
      return;
    }
  #endif
}

/* USB stub.
 */
 
void usb_begin() {
  fprintf(stderr,"STUB:%s\n",__func__);
}

void usb_send(const void *v,int c) {

  const char *src=v;
  int srcc=c;
  while (srcc&&((unsigned char)src[srcc-1]<=0x20)) srcc--;
  while (srcc&&((unsigned char)src[0]<=0x20)) { srcc--; src++; }
  if (srcc>50) srcc=0;
  int i=srcc; while (i-->0) {
    if ((src[i]<0x20)||(src[i]>0x7e)) {
      srcc=0;
      break;
    }
  }
  
  fprintf(stderr,"STUB:%s c=%d %.*s\n",__func__,c,srcc,src);
}

int usb_read(void *dst,int dsta) {
  fprintf(stderr,"STUB:%s\n",__func__);
  return -1;
}

int usb_read_byte() {
  fprintf(stderr,"STUB:%s\n",__func__);
  return -1;
}

/* Audio buffer position.
 */

int audio_estimate_buffered_frame_count() {
  #if PO_USE_alsa
    if (genioc.alsa) return alsa_estimate_buffered_frame_count(genioc.alsa);
  #endif
  return 0;
}

/* XXX print my generated waves for verification
 */
 
extern const int16_t wave0[512];

static void XXX_tempmain() {
  fprintf(stderr,"wave0:\n");
  
  int w=128,h=50; // w 128 because it's a factor of 512 (the length of the wave). h arbitrary.
  int stride=w+1;
  char *image=malloc(stride*h);
  memset(image,0x20,stride*h);
  char *nl=image+w;
  int i=h;
  for (;i-->0;nl+=stride) *nl=0x0a;
  memset(image+(h/2)*stride,'-',w);
  
  int samplespercol=512/w;
  int halfh=h/2;
  int srcp=0,dstx=0;
  for (;dstx<w;dstx++) {
    int sample=0;
    for (i=samplespercol;i-->0;srcp++) sample+=wave0[srcp];
    sample/=samplespercol;
    int y=(sample*halfh)/32768+halfh;
    y=h-y;
    if (y<0) y=0;
    else if (y>=h) y=h-1;
    image[y*stride+dstx]='X';
  }
  
  fprintf(stderr,"%.*s",(w+1)*h,image);
  free(image);
}

/* Main.
 */

int main(int argc,char **argv) {
  //XXX_tempmain();
  //return 0;
  if (genioc_argv_get_boolean(argc,argv,"--help")) {
    genioc_print_help(argv[0]);
    return 0;
  }
  if (genioc_init_drivers(argc,argv)<0) {
    fprintf(stderr,"Failed to initialize drivers.\n");
    return 1;
  }
  
  setup();
  
  int framec=0;
  const int64_t frametime=1000000/60;
  int64_t nexttime=now_us();
  int64_t starttime=nexttime;
  while (!genioc.terminate&&!genioc.sigc) {
  
    int64_t now=now_us();
    while (now<nexttime) {
      int64_t sleeptime=nexttime-now;
      if (sleeptime>100000) {
        nexttime=now;
        break;
      }
      if (sleeptime>0) sleep_us(sleeptime);
      now=now_us();
    }
    nexttime+=frametime;
    framec++;
    
    #if PO_USE_alsa
      if (alsa_lock(genioc.alsa)<0) {
        fprintf(stderr,"ERROR! Failed to acquire ALSA lock.\n");
        return 1;
      }
    #endif
    loop();
    #if PO_USE_alsa
      alsa_unlock(genioc.alsa);
    #endif
    genioc_finish_video_frame();
  }
  
  if (framec>0) {
    double elapsed=(nexttime-starttime)/1000000.0;
    fprintf(stderr,"%d video frames in %.03fs, average %.03f Hz\n",framec,elapsed,framec/elapsed);
  }
  
  genioc_quit_drivers();
  fprintf(stderr,"Normal exit.\n");
  return 0;
}
