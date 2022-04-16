#include "sb_internal.h"

/* Parse path for devid.
 */
 
int sb_devid_from_path(const char *path) {
  if (!path) return 0;
  if (memcmp(path,"/dev/ttyACM",11)) return 0;
  int p=11,id=0;
  for (;path[p];p++) {
    if (path[p]<'0') return 0;
    if (path[p]>'9') return 0;
    id*=10;
    id+=path[p]-'0';
  }
  if (!id) return 1000; // we can't use '0' but linux does
  return id;
}

/* Search.
 */
 
int sb_device_search_devid(int devid) {
  int lo=0,hi=sb.devicec;
  while (lo<hi) {
    int ck=(lo+hi)>>1;
         if (devid<sb.devicev[ck].devid) hi=ck;
    else if (devid>sb.devicev[ck].devid) lo=ck+1;
    else return ck;
  }
  return -lo-1;
}

/* Unused id.
 */
 
static int sb_unused_devid() {
  if (sb.devicec<1) return 1;
  int highest=sb.devicev[sb.devicec-1].devid;
  if (highest<INT_MAX) return highest+1;
  int id=1,p=0;
  while (1) {
    if (p>=sb.devicec) return id;
    if (sb.devicev[p].devid!=id) return id;
    id++;
    p++;
  }
}

/* Add device.
 */
 
struct sb_device *sb_device_add(int devid,const char *path,int fd) {

  if (devid<1) devid=sb_unused_devid();
  
  int p=sb_device_search_devid(devid);
  if (p>=0) return 0;
  p=-p-1;
  
  if (sb.devicec>=sb.devicea) {
    int na=sb.devicea+8;
    if (na>INT_MAX/sizeof(struct sb_device)) return 0;
    void *nv=realloc(sb.devicev,sizeof(struct sb_device)*na);
    if (!nv) return 0;
    sb.devicev=nv;
    sb.devicea=na;
  }
  
  struct sb_device *device=sb.devicev+p;
  memmove(device+1,device,sizeof(struct sb_device)*(sb.devicec-p));
  sb.devicec++;
  memset(device,0,sizeof(struct sb_device));
  device->devid=devid;
  if (path) device->path=strdup(path);
  device->fd=fd;
  
  return device;
}

/* Other searches.
 */
 
struct sb_device *sb_device_by_fd(int fd) {
  struct sb_device *device=sb.devicev;
  int i=sb.devicec;
  for (;i-->0;device++) if (device->fd==fd) return device;
  return 0;
}

/* Remove by fd.
 */
 
void sb_device_remove_fd(int fd) {
  int i=sb.devicec;
  while (i-->0) {
    struct sb_device *device=sb.devicev+i;
    if (device->fd==fd) {
      if (device->path) free(device->path);
      close(fd);
      sb.devicec--;
      memmove(device,device+1,sizeof(struct sb_device)*(sb.devicec-i));
      return;
    }
  }
}

/* Event.
 */
 
int sb_device_event(struct sb_device *device,const char *src,int srcc) {
  if (!src) srcc=0; else if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  fprintf(stderr,"%s:%s: '%.*s'\n",__func__,device->path,srcc,src);
  return 0;
}
