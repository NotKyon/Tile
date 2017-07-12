#ifndef TILE_LIGHT_H
#define TILE_LIGHT_H

#include "const.h"
#include "math.h"

TILE_EXTRNC_ENTER

/*
 * -----
 * Light
 * -----
 */
typedef enum {
	kTlLT_Point,
	kTlLT_Dir,
	kTlLT_Spot
} TlLightType_t;
typedef struct TL_CACHELINE_ALIGNED TlLight_s {
	TlVec3 xyz;
	TlVec3 dir;
	float radius, intensity;
	float innerCone, outerCone;
	TlColor color;

	struct TlEntity_s *ent;
	struct TlLight_s *prev, *next;
} TlLight;

TILE_EXTRNC_LEAVE

#endif

