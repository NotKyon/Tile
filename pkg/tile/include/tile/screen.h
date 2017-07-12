#ifndef TILE_SCREEN_H
#define TILE_SCREEN_H

#include "const.h"

TILE_EXTRNC_ENTER

/*
 * ------------------
 * Screen Description
 * ------------------
 */
typedef struct TlScreen_s {
	int width;
	int height;
	int r, g, b, a;
	int depth;
	int stencil;
} TlScreen;

/* Screen */
void tlScr_Init(TlScreen *desc);
void tlScr_Fini();
TlBool tlScr_IsOpen();

TILE_EXTRNC_LEAVE

#endif

