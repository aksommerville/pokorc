#include "fiddle_internal.h"
#include <math.h>

/* Generate and quantize sine wave.
 */
 
static void generate_sine(int16_t *v,int c) {
  if (c<1) return;
  const double peak=8000.0;
  double t=0.0,dt=(M_PI*2.0)/c;
  for (;c-->0;v++,t+=dt) {
    *v=(int16_t)(sin(t)*peak);
  }
}

/* Allocate all 8 waves and initialize to sines.
 */
 
int fiddle_make_default_waves() {
  
  int16_t **v=fiddle.wavev;
  int i=8;
  for (;i-->0;v++) {
    if (*v) free(*v);
    if (!(*v=malloc(512*sizeof(int16_t)))) return -1;
  }
  
  generate_sine(fiddle.wavev[0],512);
  for (i=1;i<8;i++) memcpy(fiddle.wavev[i],fiddle.wavev[0],sizeof(int16_t)*512);
  
  memcpy(fiddle.synth.wavev,fiddle.wavev,sizeof(fiddle.wavev));
  return 0;
}

/* If it looks like a wave source, decode and load it.
 */
 
int fiddle_wave_possible_change(const char *path,const char *base) {

  // The interesting files are all named "waveN.wave" 0..7
  int basec=0;
  while (base[basec]) basec++;
  if (basec!=10) return 0;
  int n=base[4]-'0';
  if ((n<0)||(n>7)) return 0;
  
  char cmd[256];
  int cmdc=snprintf(cmd,sizeof(cmd),"out/tool/mkwave --STDOUT=BINARY %s",path);
  if ((cmdc<1)||(cmdc>=sizeof(cmd))) return 0;
  
  FILE *child=popen(cmd,"r");
  if (!child) {
    fprintf(stderr,"%s: Failed to open child process `%s`\n",path,cmd);
    return 0;
  }
  
  int16_t tmp[512];
  int err=fread(tmp,1,1024,child);
  if (err!=1024) {
    fprintf(stderr,"%s: Got unexpected length %d from mkwave. Expected 1024.\n",path,err);
  } else {
    memcpy(fiddle.wavev[n],tmp,sizeof(tmp));
    fprintf(stderr,"%s: Replaced wave %d\n",path,n);
  }
  
  pclose(child);
  return 0;
}
