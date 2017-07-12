#ifndef TILE_SHAPES_H
#define TILE_SHAPES_H

#include "const.h"

TILE_EXTRNC_ENTER

struct TlEntity_s;
struct TlBrush_s;

/* Primitives */
struct TlEntity_s *tlNewTriangle(struct TlBrush_s *brush);
struct TlEntity_s *tlNewPlane(struct TlBrush_s *brush, float w, float h);
struct TlEntity_s *tlNewBox(struct TlBrush_s *brush, float w, float h, float d);
struct TlEntity_s *tlNewCube(struct TlBrush_s *brush, float r);
struct TlEntity_s *tlNewFigureEightTorus(struct TlBrush_s *brush, float c, TlUInt segs);

TILE_EXTRNC_LEAVE

#endif

