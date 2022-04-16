#include "sb_internal.h"
#include <signal.h>

struct sb sb={0};

/* Quit.
 */
 
static void sb_quit() {
  poller_del(sb.poller);
  inotify_del(sb.inotify);
  
  if (sb.devicev) {
    struct sb_device *device=sb.devicev;
    int i=sb.devicec;
    for (;i-->0;device++) {
      if (device->fd>=0) close(device->fd);
      if (device->path) free(device->path);
    }
    free(sb.devicev);
  }
}

/* Signal. 
 */
 
static void sb_cb_signal(int sigid) {
  switch (sigid) {
    case SIGINT: if (++(sb.sigc)>=3) {
        fprintf(stderr,"Too many unprocessed signals.\n");
        exit(1);
      } break;
  }
}

/* Event from device.
 */
 
static int sb_cb_device_error(int fd,void *userdata) {
  fprintf(stderr,"%s fd=%d\n",__func__,fd);
  poller_remove_file(sb.poller,fd);
  sb_device_remove_fd(fd);
  return 0;
}
 
static int sb_cb_device_read(int fd,void *userdata,const void *src,int srcc) {
  struct sb_device *device=sb_device_by_fd(fd);
  if (!device) return -1;
  
  const uint8_t *SRC=src;
  int srcp=0;
  while (srcp<srcc) {
    if (SRC[srcp]<=0x20) { srcp++; continue; }
    const char *line=(char*)SRC+srcp;
    int linec=0;
    while ((srcp<srcc)&&(SRC[srcp]!=0x0a)) { srcp++; linec++; }
    while (linec&&((unsigned char)line[linec-1]<=0x20)) linec--;
    if (linec) {
      if (sb_device_event(device,line,linec)<0) return -1;
    }
  }
  
  return 0;
}

/* Event from inotify.
 */
 
static int sb_cb_inotify(const char *path,const char *base,int wd,void *userdata) {
  //fprintf(stderr,"%s %s\n",__func__,path);
  int devid=sb_devid_from_path(path);
  if (devid<1) return 0;
  int p=sb_device_search_devid(devid);
  if (p>=0) return 0;
  
  int fd=open(path,O_RDONLY);
  if (fd<0) return 0;
  
  struct sb_device *device=sb_device_add(devid,path,fd);
  if (!device) {
    close(fd);
    return -1;
  }
  
  struct poller_file file={
    .fd=fd,
    .cb_error=sb_cb_device_error,
    .cb_read=sb_cb_device_read,
  };
  if (poller_add_file(sb.poller,&file)<0) {
    close(fd);
    return -1;
  }
  fprintf(stderr,"%s: Added file %d\n",path,fd);
  
  return 0;
}

static int sb_cb_inotify_readable(int fd,void *userdata) {
  return inotify_read(sb.inotify);
}

/* Init.
 */
 
static int sb_init(int argc,char **argv) {

  signal(SIGINT,sb_cb_signal);

  if (!(sb.poller=poller_new())) return -1;
  
  if (!(sb.inotify=inotify_new(sb_cb_inotify,0))) return -1;
  struct poller_file inofile={
    .fd=inotify_get_fd(sb.inotify),
    .cb_readable=sb_cb_inotify_readable,
  };
  if (poller_add_file(sb.poller,&inofile)<0) return -1;
  if (inotify_watch(sb.inotify,"/dev")<0) return -1;
  
  if (inotify_scan(sb.inotify)<0) return -1;
  
  return 0;
}

/* Update. Zero to quit.
 */
 
static int sb_update() {
  if (sb.sigc) return 0;
  if (poller_update(sb.poller,100)<0) return -1;
  return 1;
}

/* Main.
 */
 
int main(int argc,char **argv) {
  fprintf(stderr,"%s: Startup...\n",argv[0]);
  if (sb_init(argc,argv)<0) return -1;
  int err;
  fprintf(stderr,"%s: Running. SIGINT to quit...\n",argv[0]);
  while ((err=sb_update())>0) ;
  sb_quit();
  if (err<0) {
    fprintf(stderr,"%s: Terminate due to error\n",argv[0]);
    return 1;
  } else {
    fprintf(stderr,"%s: Normal exit\n",argv[0]);
    return 0;
  }
}
