#include <tile.h>

/*
 * ==========================================================================
 *
 *	SURFACE FUNCTIONS
 *
 * ==========================================================================
 */

TlSurface *tlNewSurface(TlEntity *ent) {
	TlSurface *surf;

	surf = tlAllocStruct(TlSurface);

	surf->numVerts = 0;
	surf->maxVerts = 0;
	surf->verts = (TlVertex *)0;

	surf->numInds = 0;
	surf->maxInds = 0;
	surf->inds = (unsigned short *)0;

	surf->numPasses = 0;
	surf->passes = (TlBrush **)0;

	surf->ent = ent;

	surf->s_next = (TlSurface *)0;
	if ((surf->s_prev = ent->s_tail) != (TlSurface *)0)
		ent->s_tail->s_next = surf;
	else
		ent->s_head = surf;
	ent->s_tail = surf;

	return surf;
}
TlSurface *tlDeleteSurface(TlSurface *surf) {
	int i;

	if (!surf)
		return (TlSurface *)0;

	surf->verts = (TlVertex *)tlMemory((void *)surf->verts, 0);
	surf->inds = (unsigned short *)tlMemory((void *)surf->inds, 0);

	for(i=0; i<surf->numPasses; i++)
		tlDeleteBrush(surf->passes[i]);

	surf->passes = (TlBrush **)tlMemory((void *)surf->passes, 0);

	if (surf->s_prev)
		surf->s_prev->s_next = surf->s_next;
	if (surf->s_next)
		surf->s_next->s_prev = surf->s_prev;

	if (surf->ent->s_head==surf)
		surf->ent->s_head = surf->s_next;
	if (surf->ent->s_tail==surf)
		surf->ent->s_tail = surf->s_prev;

	return (TlSurface *)tlMemory((void *)surf, 0);
}

TlVertex *tlAddSurfaceVertices(TlSurface *surf, unsigned short numVerts) {
#define VERT_GRAN 8
	size_t n;

	if (surf->numVerts + numVerts > surf->maxVerts) {
		surf->maxVerts  = surf->numVerts + numVerts;
		surf->maxVerts -= surf->maxVerts%VERT_GRAN;
		surf->maxVerts += VERT_GRAN;

		n = surf->maxVerts*sizeof(TlVertex);

		surf->verts = (TlVertex *)tlMemory((void *)surf->verts, n);
	}

	n = surf->numVerts;
	surf->numVerts += numVerts;

	return &surf->verts[n];
#undef VERT_GRAN
}
unsigned short *tlAddSurfaceTriangles(TlSurface *surf, unsigned int numTris) {
#define IND_GRAN 16
	size_t n;
	int numInds;

	numInds = numTris*3;

	if (surf->numInds + numInds > surf->maxInds) {
		surf->maxInds  = surf->numInds + numInds;
		surf->maxInds -= surf->numInds%IND_GRAN;
		surf->maxInds += IND_GRAN;

		n = surf->maxInds*sizeof(unsigned short);

		surf->inds = (unsigned short *)tlMemory((void *)surf->inds, n);
	}

	n = surf->numInds;
	surf->numInds += numInds;

	return &surf->inds[n];
#undef IND_GRAN
}

void tlAddSurfacePass(TlSurface *surf, TlBrush *brush) {
	size_t n;

	if (!brush)
		brush = tlFirstBrush();

	n = (surf->numPasses + 1)*sizeof(TlBrush *);

	surf->passes = (TlBrush **)tlMemory((void *)surf->passes, n);

	surf->passes[surf->numPasses++] = brush;
	brush->refCnt++;
}

TlSurface *tlSurfaceBefore(const TlSurface *surf) {
	return surf->s_prev;
}
TlSurface *tlSurfaceAfter(const TlSurface *surf) {
	return surf->s_next;
}
TlSurface *tlFirstSurface(const TlEntity *ent) {
	return ent->s_head;
}
TlSurface *tlLastSurface(const TlEntity *ent) {
	return ent->s_tail;
}

unsigned short tlGetSurfaceVertexCount(const TlSurface *surf) {
	return surf->numVerts;
}
unsigned short tlGetSurfaceVertexCapacity(const TlSurface *surf) {
	return surf->maxVerts;
}
TlVertex *tlGetSurfaceVertex(const TlSurface *surf, unsigned short i) {
	return &surf->verts[i];
}

unsigned int tlGetSurfaceTriangleCount(const TlSurface *surf) {
	return surf->numInds*3;
}
unsigned int tlGetSurfaceTriangleCapacity(const TlSurface *surf) {
	return surf->maxInds*3;
}
unsigned short *tlGetSurfaceTriangle(const TlSurface *surf, unsigned int i) {
	return &surf->inds[i*3];
}

unsigned int tlGetSurfacePassCount(const TlSurface *surf) {
	return surf->numPasses;
}
TlBrush *tlGetSurfacePass(const TlSurface *surf, unsigned int i) {
	return surf->passes[i];
}

void tlSetVertexPosition(TlVertex *vert, float x, float y, float z) {
	vert->xyz[0] = x;
	vert->xyz[1] = y;
	vert->xyz[2] = z;
}
void tlSetVertexNormal(TlVertex *vert, float x, float y, float z) {
	vert->norm[0] = x;
	vert->norm[1] = y;
	vert->norm[2] = z;
}
void tlSetVertexTexCoord(TlVertex *vert, float s, float t) {
	vert->st[0] = s;
	vert->st[1] = t;
}
void tlSetVertexBinormal(TlVertex *vert, float x, float y, float z) {
	vert->binorm[0] = x;
	vert->binorm[1] = y;
	vert->binorm[2] = z;
}
void tlSetVertexTangent(TlVertex *vert, float x, float y, float z) {
	vert->tangent[0] = x;
	vert->tangent[1] = y;
	vert->tangent[2] = z;
}

static unsigned char FloatToChannel(float f) {
	return (unsigned char)(f*255.0f);
}
void tlSetVertexColor3f(TlVertex *vert, float r, float g, float b) {
	vert->color[0] = FloatToChannel(r);
	vert->color[1] = FloatToChannel(g);
	vert->color[2] = FloatToChannel(b);
	vert->color[3] = 255;
}
void tlSetVertexColor4f(TlVertex *vert, float r, float g, float b, float a) {
	vert->color[0] = FloatToChannel(r);
	vert->color[1] = FloatToChannel(g);
	vert->color[2] = FloatToChannel(b);
	vert->color[3] = FloatToChannel(a);
}
void tlSetVertexColor3fv(TlVertex *vert, float *rgb) {
	vert->color[0] = FloatToChannel(rgb[0]);
	vert->color[1] = FloatToChannel(rgb[1]);
	vert->color[2] = FloatToChannel(rgb[2]);
	vert->color[3] = 255;
}
void tlSetVertexColor4fv(TlVertex *vert, float *rgba) {
	vert->color[0] = FloatToChannel(rgba[0]);
	vert->color[1] = FloatToChannel(rgba[1]);
	vert->color[2] = FloatToChannel(rgba[2]);
	vert->color[3] = FloatToChannel(rgba[3]);
}
void tlSetVertexColor(TlVertex *vert, unsigned int AARRGGBB) {
	vert->color[0] = (AARRGGBB&0x00FF0000)>>16;
	vert->color[1] = (AARRGGBB&0x0000FF00)>>8;
	vert->color[2] = (AARRGGBB&0x000000FF)>>0;
	vert->color[3] = (AARRGGBB&0xFF000000)>>24;
}

float tlGetVertexPositionX(const TlVertex *vert) {
	return vert->xyz[0];
}
float tlGetVertexPositionY(const TlVertex *vert) {
	return vert->xyz[1];
}
float tlGetVertexPositionZ(const TlVertex *vert) {
	return vert->xyz[2];
}

float tlGetVertexNormalX(const TlVertex *vert) {
	return vert->norm[0];
}
float tlGetVertexNormalY(const TlVertex *vert) {
	return vert->norm[1];
}
float tlGetVertexNormalZ(const TlVertex *vert) {
	return vert->norm[2];
}

float tlGetVertexTexCoordS(const TlVertex *vert) {
	return vert->st[0];
}
float tlGetVertexTexCoordT(const TlVertex *vert) {
	return vert->st[1];
}

float tlGetVertexBinormalX(const TlVertex *vert) {
	return vert->binorm[0];
}
float tlGetVertexBinormalY(const TlVertex *vert) {
	return vert->binorm[1];
}
float tlGetVertexBinormalZ(const TlVertex *vert) {
	return vert->binorm[2];
}

float tlGetVertexTangentX(const TlVertex *vert) {
	return vert->tangent[0];
}
float tlGetVertexTangentY(const TlVertex *vert) {
	return vert->tangent[1];
}
float tlGetVertexTangentZ(const TlVertex *vert) {
	return vert->tangent[2];
}

