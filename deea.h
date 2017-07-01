#ifndef _DEEA_H
#define _DEEA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SAFE_FREE(v) {  if (v) {free(v); v = NULL;}  }

#define DEEA_VER "0.0.1"

void init(void);

#endif




