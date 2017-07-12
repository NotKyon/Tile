#ifndef TILE_VIEW_H
#define TILE_VIEW_H

#include "const.h"
#include "math.h"

TILE_EXTRNC_ENTER

typedef enum
{
	kTlAspect_None,
	kTlAspect_Fit,
	kTlAspect_Fill
} TlAspect_t;

/*
 * ----
 * View
 * ----
 * A view from which the scene is rendered.
 */
struct TlEntity_s;
typedef struct TL_CACHELINE_ALIGNED TlView_s {
	TlMat4 M; /*projection*/
	union {
		struct {
			float fov;
		} persp;
		struct {
			float left, top, right, bottom;
		} ortho;
	} proj;
	TlBool isOrtho;
	TlBool recalcProj;
	float zn, zf;

	struct {
		float color[4];
		float depth;
		unsigned char stencil;

		TlBool clearColor:1;
		TlBool clearDepth:1;
		TlBool clearStencil:1;
	} clear;

	TlBool useAutoVP;
	float vpAuto[4];

	TlAspect_t aspectMode;
	double aspectRatio;

	int vpReal[4];

	/*
	 * TODO: Render targets can be handled here
	 */

	struct TlEntity_s *ent;
	struct TlView_s *prev, *next;
} TlView;

/* View */
TlView *tlNewView(struct TlEntity_s *ent);
TlView *tlDeleteView(TlView *v);
void tlDeleteAllViews();

void tlSetViewMatrix(TlView *v, const TlMat4 *M);
const TlMat4 *tlGetViewMatrix(TlView *v);

void tlRecalcViewport(TlView *v, int w, int h);
void tlRecalcAllViewports(int w, int h);

int tlGetViewX(const TlView *v);
int tlGetViewY(const TlView *v);
int tlGetViewWidth(const TlView *v);
int tlGetViewHeight(const TlView *v);

void tlSetViewPerspective(TlView *v, float fov);
void tlSetViewOrtho(TlView *v, float left, float top, float right, float bottom);
void tlSetViewRange(TlView *v, float zn, float zf);

float tlGetViewPerspectiveFOV(const TlView *v);
float tlGetViewOrthoLeft(const TlView *v);
float tlGetViewOrthoTop(const TlView *v);
float tlGetViewOrthoRight(const TlView *v);
float tlGetViewOrthoBottom(const TlView *v);
float tlGetViewRangeNear(const TlView *v);
float tlGetViewRangeFar(const TlView *v);

void tlEnableViewClearColor(TlView *v);
void tlDisableViewClearColor(TlView *v);
TlBool tlIsViewClearColorEnabled(const TlView *v);

void tlEnableViewClearDepth(TlView *v);
void tlDisableViewClearDepth(TlView *v);
TlBool tlIsViewClearDepthEnabled(const TlView *v);

void tlEnableViewClearStencil(TlView *v);
void tlDisableViewClearStencil(TlView *v);
TlBool tlIsViewClearStencilEnabled(const TlView *v);

void tlSetViewClearColor(TlView *v, float r, float g, float b, float a);
float tlGetViewClearColorR(const TlView *v);
float tlGetViewClearColorG(const TlView *v);
float tlGetViewClearColorB(const TlView *v);
float tlGetViewClearColorA(const TlView *v);

void tlSetViewClearDepth(TlView *v, float z);
float tlGetViewClearDepth(const TlView *v);

void tlSetViewClearStencil(TlView *v, TlU8 mask);
unsigned char tlGetViewClearStencil(const TlView *v);

void tlSetViewAutoVP(TlView *vp, float l, float t, float r, float b);
void tlSetViewRealVP(TlView *vp, int l, int t, int r, int b);
void tlSetViewAutoAspect(TlView *v, double ratio, TlAspect_t mode);

TlView *tlViewBefore(const TlView *v);
TlView *tlViewAfter(const TlView *v);
TlView *tlFirstView();
TlView *tlLastView();

TILE_EXTRNC_LEAVE

#endif

