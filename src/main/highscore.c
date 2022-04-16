#include "highscore.h"
#include "platform.h"

#if PO_NATIVE
  #include <stdlib.h>
  #include <stdio.h>
  #include <fcntl.h>
  #include <unistd.h>
  #include <sys/stat.h>
  
#else
  #include "tinysd.h"
#endif

/* Globals.
 */
 
/* Each record is:
 *   u16 songid
 *   u8 medal
 *   u24 score
 * big-endian
 */
#define RECORD_SIZE 6
#define RECORD_LIMIT 10 /* Needn't be any more than the count of included songs. */
static uint8_t highscore_content[RECORD_SIZE*RECORD_LIMIT];
static uint8_t highscore_count=0; // in records

/* Path for native builds.
 */
 
#if PO_NATIVE
  static char highscore_path_storage[1024];
  static const char *highscore_path() {
    const char *home=getenv("HOME");
    int c=snprintf(highscore_path_storage,sizeof(highscore_path_storage),"%s/.pocket-orchestra/highscore",home);
    if ((c<1)||(c>=sizeof(highscore_path_storage))) return 0;
    int slashp=c-1;
    while (slashp&&(highscore_path_storage[slashp]!='/')) slashp--;
    highscore_path_storage[slashp]=0;
    mkdir(highscore_path_storage,0775);
    highscore_path_storage[slashp]='/';
    return highscore_path_storage;
  }
#endif

/* Read from disk.
 */
 
static void highscore_load() {
  #if PO_NATIVE
    const char *path=highscore_path();
    if (!path) return;
    int fd=open(path,O_RDONLY);
    if (fd<0) return;
    int len=read(fd,highscore_content,sizeof(highscore_content));
    close(fd);
  #else
    int32_t len=tinysd_read(highscore_content,sizeof(highscore_content),"/Pokorc/hiscore.bin");
  #endif
  if (len<0) len=0;
  highscore_count=len/RECORD_SIZE;
}

/* Write to disk.
 */
 
static void highscore_save() {
  #if PO_NATIVE
    const char *path=highscore_path();
    if (!path) return;
    int fd=open(path,O_WRONLY|O_CREAT|O_TRUNC,0666);
    if (fd<0) return;
    write(fd,highscore_content,RECORD_SIZE*highscore_count);
    close(fd);
  #else
    tinysd_write("/Pokorc/hiscore.bin",highscore_content,RECORD_SIZE*highscore_count);
  #endif
}

/* Get one high score.
 */
 
void highscore_get(uint32_t *score,uint8_t *medal,uint16_t songid) {
  if (!highscore_count) highscore_load();
  const uint8_t *src=highscore_content;
  uint8_t i=highscore_count;
  for (;i-->0;src+=RECORD_SIZE) {
    uint16_t id=(src[0]<<8)|src[1];
    if (id!=songid) continue;
    *medal=src[2];
    *score=(src[3]<<16)|(src[4]<<8)|src[5];
    return;
  }
  *medal=0;
  *score=0;
}

/* Set one high score.
 */
 
void highscore_set(uint16_t songid,uint32_t score,uint8_t medal) {
  if (!highscore_count) highscore_load();
  uint8_t *src=highscore_content;
  uint8_t i=highscore_count;
  for (;i-->0;src+=RECORD_SIZE) {
    uint16_t id=(src[0]<<8)|src[1];
    if (id!=songid) continue;
    src[2]=medal;
    src[3]=score>>16;
    src[4]=score>>8;
    src[5]=score;
    highscore_save();
    return;
  }
  if (highscore_count>=RECORD_LIMIT) return; // oh no
  src[0]=songid>>8;
  src[1]=songid;
  src[2]=medal;
  src[3]=score>>16;
  src[4]=score>>8;
  src[5]=score;
  highscore_count++;
  highscore_save();
}

/* Send score to server.
 */
 
void highscore_send(uint16_t songid,uint32_t score,uint8_t medal) {
  char msg[64];
  int msgc=snprintf(msg,sizeof(msg),"score:%d:%d:%d\n",songid,score,medal);
  if ((msgc<1)||(msgc>=sizeof(msg))) return;
  usb_send(msg,msgc);
}
