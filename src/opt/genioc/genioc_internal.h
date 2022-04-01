#ifndef GENIOC_INTERNAL_H
#define GENIOC_INTERNAL_H

#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include "clock.h"
#include "main/platform.h"

#if PO_USE_x11
  #include "opt/x11/po_x11.h"
#endif

extern struct genioc {
  #if PO_USE_x11
    struct po_x11 *x11;
  #endif
  int terminate;
  uint8_t inputstate;
  volatile int sigc;
} genioc;

#endif
