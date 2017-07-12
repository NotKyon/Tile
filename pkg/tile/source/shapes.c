#include <tile/shapes.h>
#include <tile/entity.h>
#include <tile/math.h>
#include <tile/brush.h>
#include <tile/surface.h>

/*
 * ==========================================================================
 *
 *	PRIMITIVES
 *
 * ==========================================================================
 */
TlEntity *tlNewTriangle(TlBrush *brush) {
	unsigned short *tri;
	TlSurface *surf;
	TlEntity *ent;
	TlVertex *verts;

	ent = tlNewEntity((TlEntity *)0);
	surf = tlNewSurface(ent);

	verts = tlAddSurfaceVertices(surf, 3);
	tri = tlAddSurfaceTriangles(surf, 1);

	tlSetVertexPosition(&verts[0], -0.5f,-0.5f, 0.0f);
	tlSetVertexPosition(&verts[1],  0.0f,+0.5f, 0.0f);
	tlSetVertexPosition(&verts[2], +0.5f,-0.5f, 0.0f);

	tlSetVertexNormal(&verts[0], 0,0,-1);
	tlSetVertexNormal(&verts[1], 0,0,-1);
	tlSetVertexNormal(&verts[2], 0,0,-1);

	tlSetVertexColor3f(&verts[0], 1.0f,0.0f,0.0f);
	tlSetVertexColor3f(&verts[1], 0.0f,1.0f,0.0f);
	tlSetVertexColor3f(&verts[2], 0.0f,0.0f,1.0f);

	tri[0] = 0;
	tri[1] = 1;
	tri[2] = 2;

	tlAddSurfacePass(surf, brush);

	return ent;
}

TlEntity *tlNewPlane(TlBrush *brush, float w,float h) {
	unsigned short *tris;
	TlSurface *surf;
	TlEntity *ent;
	TlVertex *verts;
	float x,y;

	ent = tlNewEntity((TlEntity *)0);
	surf = tlNewSurface(ent);

	verts = tlAddSurfaceVertices(surf, 4);
	tris = tlAddSurfaceTriangles(surf, 2);

	x = w/2.0f;
	y = h/2.0f;

	tlSetVertexPosition(&verts[0], -x,-y,0.0f);
	tlSetVertexPosition(&verts[1], -x,+y,0.0f);
	tlSetVertexPosition(&verts[2], +x,+y,0.0f);
	tlSetVertexPosition(&verts[3], +x,-y,0.0f);

	tlSetVertexNormal(&verts[0], 0.0f,0.0f,-1.0f);
	tlSetVertexNormal(&verts[1], 0.0f,0.0f,-1.0f);
	tlSetVertexNormal(&verts[2], 0.0f,0.0f,-1.0f);
	tlSetVertexNormal(&verts[3], 0.0f,0.0f,-1.0f);

	tlSetVertexColor3f(&verts[0], 0.0f,0.0f,0.0f);
	tlSetVertexColor3f(&verts[1], 0.0f,1.0f,0.0f);
	tlSetVertexColor3f(&verts[2], 1.0f,1.0f,0.0f);
	tlSetVertexColor3f(&verts[3], 1.0f,0.0f,0.0f);

	tlSetVertexTexCoord(&verts[0], 0.0f,0.0f);
	tlSetVertexTexCoord(&verts[1], 0.0f,1.0f);
	tlSetVertexTexCoord(&verts[2], 1.0f,1.0f);
	tlSetVertexTexCoord(&verts[3], 1.0f,0.0f);

	*tris++ = 0;
	*tris++ = 1;
	*tris++ = 2;

	*tris++ = 0;
	*tris++ = 2;
	*tris++ = 3;

	tlAddSurfacePass(surf, brush);

	return ent;
}

static void SetCorner(TlVertex *verts, TlUInt A, TlUInt B, TlUInt C, float x, float y, float z) {
	unsigned char r, g, b;

	tlSetVertexPosition(&verts[A], x,y,z);
	tlSetVertexPosition(&verts[B], x,y,z);
	tlSetVertexPosition(&verts[C], x,y,z);

	r = x < 0 ? 0 : 255;
	g = y < 0 ? 0 : 255;
	b = z < 0 ? 0 : 255;

	verts[A].color[0] = r; verts[A].color[1] = g; verts[A].color[2] = b; verts[A].color[3] = 255;
	verts[B].color[0] = r; verts[B].color[1] = g; verts[B].color[2] = b; verts[B].color[3] = 255;
	verts[C].color[0] = r; verts[C].color[1] = g; verts[C].color[2] = b; verts[C].color[3] = 255;
}
static void SetFaceNormal(TlVertex *verts, float x, float y, float z) {
	tlSetVertexNormal(&verts[0], x,y,z);
	tlSetVertexNormal(&verts[1], x,y,z);
	tlSetVertexNormal(&verts[2], x,y,z);
	tlSetVertexNormal(&verts[3], x,y,z);
}
TlEntity *tlNewBox(TlBrush *brush, float w, float h, float d) {
#define F_FRONT		0
#define F_LEFT		1
#define F_TOP		2
#define F_RIGHT		3
#define F_BACK		4
#define F_BOTTOM	5
#define I_FRONT		(F_FRONT*4)
#define I_LEFT		(F_LEFT*4)
#define I_TOP		(F_TOP*4)
#define I_RIGHT		(F_RIGHT*4)
#define I_BACK		(F_BACK*4)
#define I_BOTTOM	(F_BOTTOM*4)
	unsigned short *tris;
	TlSurface *surf;
	TlEntity *ent;
	TlVertex *verts;
	float x,y,z;
	TlUInt i;

	/*
	
		front			left		top				right			back			bottom
		1,2,3,4,		5,6,7,8,	9,10,11,12,		13,14,15,16,	17,18,19,20,	21,22,23,24

		tlf				trf			blf				brf
		1,5,9			1,9,13		1,5,21			1,13,21

		tlb				trb			blb				brb
		5,9,17			9,13,17		5,17,21			13,17,21
	
	*/

	ent = tlNewEntity((TlEntity *)0);
	surf = tlNewSurface(ent);

	verts = tlAddSurfaceVertices(surf, 24);
	tris = tlAddSurfaceTriangles(surf, 12);

	x = w/2.0f;
	y = h/2.0f;
	z = d/2.0f;

	SetCorner(verts,I_LEFT +2,I_TOP   +0,I_FRONT+1, -x,+y,-z);
	SetCorner(verts,I_RIGHT+1,I_TOP   +3,I_FRONT+2, +x,+y,-z);
	SetCorner(verts,I_LEFT +3,I_BOTTOM+3,I_FRONT+0, -x,-y,-z);
	SetCorner(verts,I_RIGHT+0,I_BOTTOM+0,I_FRONT+3, +x,-y,-z);
	SetCorner(verts,I_LEFT +1,I_TOP   +1,I_BACK +2, -x,+y,+z);
	SetCorner(verts,I_RIGHT+2,I_TOP   +2,I_BACK +1, +x,+y,+z);
	SetCorner(verts,I_LEFT +0,I_BOTTOM+2,I_BACK +3, -x,-y,+z);
	SetCorner(verts,I_RIGHT+3,I_BOTTOM+1,I_BACK +0, +x,-y,+z);

	SetFaceNormal(&verts[I_FRONT ], 0,0,-z);
	SetFaceNormal(&verts[I_BACK  ], 0,0,+z);
	SetFaceNormal(&verts[I_TOP   ], 0,+y,0);
	SetFaceNormal(&verts[I_BOTTOM], 0,-y,0);
	SetFaceNormal(&verts[I_LEFT  ], -x,0,0);
	SetFaceNormal(&verts[I_RIGHT ], +x,0,0);

	for(i=0; i<6; i++) {
		tlSetVertexTexCoord(&verts[i*4 + 0], 0.0f,0.0f);
		tlSetVertexTexCoord(&verts[i*4 + 1], 0.0f,1.0f);
		tlSetVertexTexCoord(&verts[i*4 + 2], 1.0f,1.0f);
		tlSetVertexTexCoord(&verts[i*4 + 3], 1.0f,0.0f);

		*tris++ = i*4 + 0;
		*tris++ = i*4 + 1;
		*tris++ = i*4 + 2;
		*tris++ = i*4 + 0;
		*tris++ = i*4 + 2;
		*tris++ = i*4 + 3;
	}

	tlAddSurfacePass(surf, brush);

	return ent;
}
TlEntity *tlNewCube(TlBrush *brush, float r) {
	return tlNewBox(brush, r, r, r);
}

static TlUInt unorm_color(float r, float g, float b) {
	union { unsigned char b[4]; unsigned int u; } v;

	v.b[0] = (unsigned char)(r*255.0f);
	v.b[1] = (unsigned char)(g*255.0f);
	v.b[2] = (unsigned char)(b*255.0f);
	v.b[3] = 0xFF;

	return v.u;
}
static TlUInt snorm_color(float rgb[3]) {
	return unorm_color((rgb[0] + 1)/2, (rgb[1] + 1)/2, (rgb[2] + 1)/2);
}
static void figureEightTorus_f(float *xyz, float c, float u, float v) {
	xyz[0] = tlCos(u)*(c + tlSin(v)*tlCos(u) - tlSin(2.0f*v)*tlSin(u)/2.0f);
	xyz[1] = tlSin(u)*(c + tlSin(v)*tlCos(u) - tlSin(2.0f*v)*tlSin(u)/2.0f);
	xyz[2] = tlSin(u)*tlSin(v) + tlCos(u)*tlSin(2.0f*v)/2.0f;
}

TlEntity *tlNewFigureEightTorus(TlBrush *brush, float c, TlUInt segs) {
	unsigned short *tris, idx;
	TlSurface *surf;
	TlEntity *ent;
	TlVertex *verts;
	float vl[3], vh[3];
	float u, v, step;
	int i, j;

	if(!c) c=1.0f;
	if(!segs) segs=36;

	ent = tlNewEntity((TlEntity *)0);
	surf = tlNewSurface(ent);

	step = 360.0f / (float)segs;

	vl[0] = -1 - c;
	vl[1] = -1 - c;
	vl[2] = -1 - c;
	vh[0] = +1 + c;
	vh[1] = +1 + c;
	vh[2] = +1 + c;

	vl[0] /= 2;
	vl[1] /= 2;
	vl[2] /= 2;
	vh[0] /= 2;
	vh[1] /= 2;
	vh[2] /= 2;

	for(u=0; u<360; u+=step) {
		for(v=0; v<360; v+=step) {
			float s0, t0, s1, t1;

			s0 = u;
			t0 = v;
			s1 = u + step;
			t1 = v + step;

			idx = tlGetSurfaceVertexCount(surf);
			verts = tlAddSurfaceVertices(surf, 4);
			tris = tlAddSurfaceTriangles(surf, 2);

			figureEightTorus_f(verts[0].xyz, c, s0, t0);
			figureEightTorus_f(verts[1].xyz, c, s1, t0);
			figureEightTorus_f(verts[2].xyz, c, s0, t1);
			figureEightTorus_f(verts[3].xyz, c, s1, t1);

			for(i=0; i<4; i++) {
				float rgb[3];

				for(j=0; j<3; j++) {
					verts[i].xyz[j] /= vh[j] - vl[j];
					rgb[j] = verts[i].xyz[j];
					verts[i].xyz[j] /= 2;
				}

				*(TlUInt *)verts[i].color = snorm_color(rgb);
			}

			tris[0*3 + 0]=idx + 0; tris[0*3 + 1]=idx + 1; tris[0*3 + 2]=idx + 2;
			tris[1*3 + 0]=idx + 2; tris[1*3 + 1]=idx + 1; tris[1*3 + 2]=idx + 3;
		}
	}

	tlAddSurfacePass(surf, brush);

	return ent;
}

