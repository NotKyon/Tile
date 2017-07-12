#include <tile/view.h>
#include <tile/entity.h>
#include <tile/math.h>

static void ApplyAspect( int *pOutW, int *pOutH, int InW, int InH, double fAspectRatio, TlAspect_t Aspect )
{
	double fResX, fResY;
	double fDstAspectRatio;

	if( fAspectRatio*fAspectRatio < 0.000001 || !InW || !InH ) {
		*pOutW = InW;
		*pOutH = InH;
		return;
	}

	fResX = ( double )InW;
	fResY = ( double )InH;

	if( Aspect != kTlAspect_Fit && Aspect != kTlAspect_Fill ) {
		*pOutW = InW;
		*pOutH = InH;
		return;
	}

	fDstAspectRatio = Aspect == kTlAspect_Fit ? fResX/fResY : fResY/fResX;

	if( fDstAspectRatio > fAspectRatio ) {
		*pOutW = ( int )( fResY*fAspectRatio + 0.5 );
		*pOutH = InH;
	} else {
		*pOutW = InW;
		*pOutH = ( int )( fResX/fAspectRatio + 0.5 );
	}
}


/*
 * ==========================================================================
 *
 *	VIEW SYSTEM
 *
 * ==========================================================================
 */
TlView *g_view_head = (TlView *)0;
TlView *g_view_tail = (TlView *)0;

TlView *tlNewView(TlEntity *ent) {
	TlView *v;

	v = tlAllocStruct(TlView);

	v->proj.persp.fov = 60.0f;
	v->recalcProj = TRUE;
	v->isOrtho = FALSE;
	v->zn = 1.0f;
	v->zf = 4096.0f;

	v->clear.color[0] = 0.1f;
	v->clear.color[1] = 0.3f;
	v->clear.color[2] = 0.5f;
	v->clear.color[3] = 1.0f;
	v->clear.depth = 1.0f;
	v->clear.stencil = 0x00;

	v->clear.clearColor = TRUE;
	v->clear.clearDepth = TRUE;
	v->clear.clearStencil = TRUE;

	v->useAutoVP = TRUE;
	v->vpAuto[0] = 0.0f;
	v->vpAuto[1] = 0.0f;
	v->vpAuto[2] = 1.0f;
	v->vpAuto[3] = 1.0f;

	v->vpReal[0] = 0;
	v->vpReal[1] = 0;
	v->vpReal[2] = 0;
	v->vpReal[3] = 0;

	v->aspectMode = kTlAspect_None;
	v->aspectRatio = 1.0;

	if( ent->view != (TlView *)0 ) {
		ent->view->ent = (TlEntity *)0;
	}

	v->ent = ent;
	ent->view = v;

	v->next = (TlView *)0;
	if( (v->prev = g_view_tail) != (TlView *)0 ) {
		g_view_tail->next = v;
	} else {
		g_view_head = v;
	}
	g_view_tail = v;

	return v;
}
TlView *tlDeleteView(TlView *v) {
	if( !v ) {
		return (TlView *)0;
	}

	if( v->ent != (TlEntity *)0 ) {
		v->ent->view = (TlView *)0;
	}

	if( v->prev != (TlView *)0 ) {
		v->prev->next = v->next;
	} else {
		g_view_head = v->next;
	}

	if( v->next != (TlView *)0 ) {
		v->next->prev = v->prev;
	} else {
		g_view_tail = v->prev;
	}

	return (TlView *)tlMemory((void *)v, 0);
}
void tlDeleteAllViews() {
	while( g_view_head != (TlView *)0 ) {
		tlDeleteView( g_view_head );
	}
}

void tlSetViewMatrix(TlView *v, const TlMat4 *M) {
	memcpy(&v->M, ( const void * )M, sizeof(TlMat4));
	v->recalcProj = FALSE;
}
const TlMat4 *tlGetViewMatrix(TlView *v) {
	float l, r, b, t;
	float w, h;

	if( !v->recalcProj ) {
		return &v->M;
	}

	if (v->isOrtho) {
		l = v->proj.ortho.left;
		r = v->proj.ortho.right;
		b = v->proj.ortho.bottom;
		t = v->proj.ortho.top;

		tlLoadOrtho(&v->M, l, r, b, t, v->zn, v->zf);
	} else {
		w = (float)tlGetViewWidth(v);
		h = (float)tlGetViewHeight(v);

		tlLoadPerspective(&v->M, v->proj.persp.fov, w/h, v->zn, v->zf);
	}

	v->recalcProj = FALSE;
	return &v->M;
}
void tlRecalcViewport(TlView *v, int w, int h) {
	int resx, resy;
	int curresx, curresy;

	if( !v->useAutoVP ) {
		return;
	}

	v->vpReal[0] = (int)(v->vpAuto[0]*(float)w);
	v->vpReal[1] = (int)(v->vpAuto[1]*(float)h);
	v->vpReal[2] = (int)(v->vpAuto[2]*(float)w);
	v->vpReal[3] = (int)(v->vpAuto[3]*(float)h);

	curresx = v->vpReal[2] - v->vpReal[0];
	curresy = v->vpReal[3] - v->vpReal[1];

	ApplyAspect( &resx, &resy, curresx, curresy, v->aspectRatio, v->aspectMode );
	if( resx != curresx || resy != curresy ) {
		int centerx;
		int centery;

		centerx = v->vpReal[0] + curresx/2;
		centery = v->vpReal[1] + curresy/2;

		v->vpReal[0] = centerx - resx/2;
		v->vpReal[1] = centery - resy/2;
		v->vpReal[2] = v->vpReal[0] + resx;
		v->vpReal[3] = v->vpReal[1] + resy;
	}
}
void tlRecalcAllViewports(int w, int h) {
	TlView *v;

	for(v=g_view_head; v!=(TlView *)0; v=v->next) {
		tlRecalcViewport(v, w, h);
	}
}
int tlGetViewX(const TlView *v) {
	return v->vpReal[0];
}
int tlGetViewY(const TlView *v) {
	return v->vpReal[1];
}
int tlGetViewWidth(const TlView *v) {
	return v->vpReal[2] - v->vpReal[0];
}
int tlGetViewHeight(const TlView *v) {
	return v->vpReal[3] - v->vpReal[1];
}

void tlSetViewPerspective(TlView *v, float fov) {
	v->proj.persp.fov = fov;

	v->isOrtho = FALSE;
	v->recalcProj = TRUE;
}
void tlSetViewOrtho(TlView *v, float left, float top, float right, float bottom) {
	v->proj.ortho.left = left;
	v->proj.ortho.top = top;
	v->proj.ortho.right = right;
	v->proj.ortho.bottom = bottom;

	v->isOrtho = TRUE;
	v->recalcProj = TRUE;
}
void tlSetViewRange(TlView *v, float zn, float zf) {
	v->zn = zn;
	v->zf = zf;

	v->recalcProj = TRUE;
}

float tlGetViewPerspectiveFOV(const TlView *v) {
	return v->proj.persp.fov;
}
float tlGetViewOrthoLeft(const TlView *v) {
	return v->proj.ortho.left;
}
float tlGetViewOrthoTop(const TlView *v) {
	return v->proj.ortho.top;
}
float tlGetViewOrthoRight(const TlView *v) {
	return v->proj.ortho.right;
}
float tlGetViewOrthoBottom(const TlView *v) {
	return v->proj.ortho.bottom;
}
float tlGetViewRangeNear(const TlView *v) {
	return v->zn;
}
float tlGetViewRangeFar(const TlView *v) {
	return v->zf;
}

void tlEnableViewClearColor(TlView *v) {
	v->clear.clearColor = TRUE;
}
void tlDisableViewClearColor(TlView *v) {
	v->clear.clearColor = FALSE;
}
TlBool tlIsViewClearColorEnabled(const TlView *v) {
	return v->clear.clearColor;
}

void tlEnableViewClearDepth(TlView *v) {
	v->clear.clearDepth = TRUE;
}
void tlDisableViewClearDepth(TlView *v) {
	v->clear.clearDepth = FALSE;
}
TlBool tlIsViewClearDepthEnabled(const TlView *v) {
	return v->clear.clearDepth;
}

void tlEnableViewClearStencil(TlView *v) {
	v->clear.clearStencil = TRUE;
}
void tlDisableViewClearStencil(TlView *v) {
	v->clear.clearStencil = FALSE;
}
TlBool tlIsViewClearStencilEnabled(const TlView *v) {
	return v->clear.clearStencil;
}

void tlSetViewClearColor(TlView *v, float r, float g, float b, float a) {
	v->clear.color[0] = r;
	v->clear.color[1] = g;
	v->clear.color[2] = b;
	v->clear.color[3] = a;
}
float tlGetViewClearColorR(const TlView *v) {
	return v->clear.color[0];
}
float tlGetViewClearColorG(const TlView *v) {
	return v->clear.color[1];
}
float tlGetViewClearColorB(const TlView *v) {
	return v->clear.color[2];
}
float tlGetViewClearColorA(const TlView *v) {
	return v->clear.color[3];
}

void tlSetViewClearDepth(TlView *v, float z) {
	v->clear.depth = z;
}
float tlGetViewClearDepth(const TlView *v) {
	return v->clear.depth;
}

void tlSetViewClearStencil(TlView *v, unsigned char mask) {
	v->clear.stencil = mask;
}
unsigned char tlGetViewClearStencil(const TlView *v) {
	return v->clear.stencil;
}

void tlSetViewAutoVP(TlView *v, float l, float t, float r, float b) {
	v->useAutoVP = TRUE;
	v->vpAuto[0] = l;
	v->vpAuto[1] = t;
	v->vpAuto[2] = r;
	v->vpAuto[3] = b;
}
void tlSetViewRealVP(TlView *v, int l, int t, int r, int b) {
	v->useAutoVP = FALSE;
	v->vpReal[0] = l;
	v->vpReal[1] = t;
	v->vpReal[2] = r;
	v->vpReal[3] = b;
}

void tlSetViewAutoAspect(TlView *v, double ratio, TlAspect_t mode) {
	v->aspectRatio = ratio;
	v->aspectMode = mode;
}

TlView *tlViewBefore(const TlView *v) {
	return v->prev;
}
TlView *tlViewAfter(const TlView *v) {
	return v->next;
}
TlView *tlFirstView() {
	return g_view_head;
}
TlView *tlLastView() {
	return g_view_tail;
}

