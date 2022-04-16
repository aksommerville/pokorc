#ifndef SB_INTERNAL_H
#define SB_INTERNAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include "tool/common/poller.h"
#include "opt/inotify/inotify.h"

extern struct sb {
  struct poller *poller;
  struct inotify *inotify;
  volatile int sigc;
  
  struct sb_device {
    int devid;
    char *path;
    int fd;
  } *devicev;
  int devicec,devicea;
} sb;

// >0 if it looks like a Tiny device.
int sb_devid_from_path(const char *path);

int sb_device_search_devid(int devid);
struct sb_device *sb_device_add(int devid,const char *path,int fd);
struct sb_device *sb_device_by_fd(int fd);
void sb_device_remove_fd(int fd);

int sb_device_event(struct sb_device *device,const char *src,int srcc);

#endif
