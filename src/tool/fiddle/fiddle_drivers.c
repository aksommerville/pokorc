#include "fiddle_internal.h"

/* Quit.
 */
 
void fiddle_drivers_quit() {
  #if PO_USE_alsa
    alsa_del(fiddle.alsa);
  #endif
  #if PO_USE_inotify
    inotify_del(fiddle.inotify);
  #endif
}

/* Init.
 */
 
int fiddle_drivers_init() {

  #if PO_USE_alsa
    struct alsa_delegate alsa_delegate={
      .rate=22050,
      .chanc=1,
      .cb_pcm_out=(void*)fiddle_cb_pcm,
      .cb_midi_in=(void*)fiddle_cb_midi,
    };
    if (!(fiddle.alsa=alsa_new(&alsa_delegate))) {
      fprintf(stderr,"Failed to initialize ALSA\n");
      return -1;
    }
    fiddle.rate=alsa_get_rate(fiddle.alsa);
    fiddle.chanc=alsa_get_chanc(fiddle.alsa);
    fprintf(stderr,"Initialized ALSA: rate=%d chanc=%d\n",fiddle.rate,fiddle.chanc);
  #endif
  
  #if PO_USE_inotify
    if (!(fiddle.inotify=inotify_new(fiddle_cb_inotify,0))) {
      fprintf(stderr,"Failed to initialize inotify.\n");
    } else {
      inotify_watch(fiddle.inotify,"src/data/embed"); // TODO don't depend on working directory
      inotify_scan(fiddle.inotify);
    }
  #endif

  return 0;
}

/* Update.
 */
 
int fiddle_drivers_update() {

  #if PO_USE_alsa
    if (alsa_update(fiddle.alsa)<0) {
      fprintf(stderr,"Error updating ALSA.\n");
      return -1;
    }
    if (fiddle.pcmlock) {
      alsa_unlock(fiddle.alsa);
      fiddle.pcmlock=0;
    }
  #endif
  
  #if PO_USE_inotify
    if (fiddle.inotify) {
      struct pollfd pollfd={.fd=inotify_get_fd(fiddle.inotify),.events=POLLIN|POLLERR|POLLHUP};
      if (poll(&pollfd,1,0)>0) {
        if (inotify_read(fiddle.inotify)<0) {
          fprintf(stderr,"Error polling inotify. Shutting it down.\n");
          inotify_del(fiddle.inotify);
          fiddle.inotify=0;
        }
      }
    }
  #endif
  
  return 0;
}

/* Lock PCM.
 */
 
int fiddle_pcm_lock() {
  if (fiddle.pcmlock) return 0;
  #if PO_USE_alsa
    if (alsa_lock(fiddle.alsa)<0) return -1;
  #endif
  fiddle.pcmlock=1;
  return 0;
}
