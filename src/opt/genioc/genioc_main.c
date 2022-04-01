#include "genioc_internal.h"
#include <signal.h>

struct genioc genioc={0};

/* Quit drivers.
 */
 
static void genioc_quit_drivers() {
  #if PO_USE_x11
    po_x11_del(genioc.x11);
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

/* Init drivers.
 */
 
static int genioc_init_drivers() {

  signal(SIGINT,genioc_rcvsig);

  #if PO_USE_x11
    if (!(genioc.x11=po_x11_new(
      "Pocket Orchestra",
      96,64,0,
      genioc_cb_x11_button,
      genioc_cb_x11_close,
      &genioc
    ))) {
      fprintf(stderr,"Failed to initialize X11\n");
      return -1;
    }
  #endif
  
  return 0;
}

/* Init per client (noop).
 */
 
uint8_t platform_init() {
  return 1;
}

/* Update.
 */
 
uint8_t platform_update() {
  #if PO_USE_x11
    po_x11_update(genioc.x11);
  #endif
  return genioc.inputstate;
}

/* Receive framebuffer.
 */
 
void platform_send_framebuffer(const void *fb) {
  #if PO_USE_x11
    po_x11_swap(genioc.x11,fb);
  #endif
}

/* Main.
 */

int main(int argc,char **argv) {
  if (genioc_init_drivers()<0) {
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
    
    loop();
  }
  
  if (framec>0) {
    double elapsed=(nexttime-starttime)/1000000.0;
    fprintf(stderr,"%d video frames in %.03fs, average %.03f Hz\n",framec,elapsed,framec/elapsed);
  }
  
  genioc_quit_drivers();
  fprintf(stderr,"Normal exit.\n");
  return 0;
}
