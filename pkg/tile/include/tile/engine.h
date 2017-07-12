#ifndef TILE_ENGINE_H
#define TILE_ENGINE_H

#include "const.h"

TILE_EXTRNC_ENTER

TlBool tlInit(void);
void tlFini(void);

TlBool tlLoop(void);

double tlGetDeltaTime(void);

TILE_EXTRNC_LEAVE

#endif
