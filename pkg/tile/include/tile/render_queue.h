#ifndef TILE_RENDER_QUEUE_H
#define TILE_RENDER_QUEUE_H

#include "const.h"

TILE_EXTRNC_ENTER

struct TlMat4_s;

/*
 * ------------
 * Render Queue
 * ------------
 * The render queue performs the actual rendering commands
 */
struct TlBrush_s;
struct TlSurface_s;
struct TlEntity_s;

typedef enum {
	/* Sort back-to-front */
	kTlRQMode_BackToFront,
	/* Sort front-to-back */
	kTlRQMode_FrontToBack,
	/* Sort by least expensive to most expensive states */
	kTlRQMode_StateSorted,
	/* Render depth-only back-to-front first, then state sorted depth-equal */
	kTlRQMode_DepthPrepass
} TlRenderQueueMode_t;

#pragma pack(push, 1)
typedef struct TlDrawItem_s {
	TlU32 order; /* ( RenderTarget<<24 ) | ( Pass<<16 ) | ( Order ) */
	struct TlMat4_s *M;
	struct TlBrush_s *brush;
	struct TlSurface_s *surf;
} TlDrawItem;
#pragma pack(pop)

/* Render Queue */
void tlRQ_SetMode(TlRenderQueueMode_t mode);
TlRenderQueueMode_t tlRQ_GetMode(void);
TlDrawItem *tlRQ_AddDrawItems(size_t numItems);
void tlRQ_AddEntities(struct TlEntity_s *ent, const struct TlMat4_s *V);
int tlRQ_CmpFunc(const TlDrawItem *a, const TlDrawItem *b);
void tlRQ_Sort();
size_t tlRQ_Count();
size_t tlRQ_Capacity();
void tlRQ_Draw();

TILE_EXTRNC_LEAVE

#endif

