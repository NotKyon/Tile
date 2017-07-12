#ifndef TILE_SURFACE_H
#define TILE_SURFACE_H

#include "const.h"

TILE_EXTRNC_ENTER

/*
 * -------
 * Surface
 * -------
 * Surfaces are rendered via Entities. They use brushes to define their
 * appearance.
 */
struct TlEntity_s;
struct TlBrush_s;
struct TlSurface_s;

typedef struct TlVertex_s {
	float xyz[3];
	float norm[3];
	float st[2];
	float binorm[3];
	float tangent[3];
	unsigned char color[4];
} TlVertex;
typedef struct TL_CACHELINE_ALIGNED TlSurface_s {
	unsigned short numVerts, maxVerts;
	TlVertex *verts;

	int numInds, maxInds;
	unsigned short *inds;

	int numPasses;
	struct TlBrush_s **passes;

	struct TlEntity_s *ent;
	struct TlSurface_s *s_prev, *s_next;
} TlSurface;

/* Surface */
struct TlSurface_s *tlNewSurface(struct TlEntity_s *ent);
struct TlSurface_s *tlDeleteSurface(struct TlSurface_s *surf);

TlVertex *tlAddSurfaceVertices(struct TlSurface_s *surf, unsigned short numVerts);
unsigned short *tlAddSurfaceTriangles(struct TlSurface_s *surf, unsigned int numTris);

void tlAddSurfacePass(struct TlSurface_s *surf, struct TlBrush_s *brush);

struct TlSurface_s *tlSurfaceBefore(const struct TlSurface_s *surf);
struct TlSurface_s *tlSurfaceAfter(const struct TlSurface_s *surf);
struct TlSurface_s *tlFirstSurface(const struct TlEntity_s *ent);
struct TlSurface_s *tlLastSurface(const struct TlEntity_s *ent);

unsigned short tlGetSurfaceVertexCount(const struct TlSurface_s *surf);
unsigned short tlGetSurfaceVertexCapacity(const struct TlSurface_s *surf);
TlVertex *tlGetSurfaceVertex(const struct TlSurface_s *surf, unsigned short i);

unsigned int tlGetSurfaceTriangleCount(const struct TlSurface_s *surf);
unsigned int tlGetSurfaceTriangleCapacity(const struct TlSurface_s *surf);
unsigned short *tlGetSurfaceTriangle(const struct TlSurface_s *surf, unsigned int i);

unsigned int tlGetSurfacePassCount(const struct TlSurface_s *surf);
struct TlBrush_s *tlGetSurfacePass(const struct TlSurface_s *surf, unsigned int i);

void tlSetVertexPosition(TlVertex *vert, float x, float y, float z);
void tlSetVertexNormal(TlVertex *vert, float x, float y, float z);
void tlSetVertexTexCoord(TlVertex *vert, float s, float t);
void tlSetVertexBinormal(TlVertex *vert, float x, float y, float z);
void tlSetVertexTangent(TlVertex *vert, float x, float y, float z);

void tlSetVertexColor3f(TlVertex *vert, float r, float g, float b);
void tlSetVertexColor4f(TlVertex *vert, float r, float g, float b, float a);
void tlSetVertexColor3fv(TlVertex *vert, float *rgb);
void tlSetVertexColor4fv(TlVertex *vert, float *rgba);
void tlSetVertexColor(TlVertex *vert, unsigned int AARRGGBB);

float tlGetVertexPositionX(const TlVertex *vert);
float tlGetVertexPositionY(const TlVertex *vert);
float tlGetVertexPositionZ(const TlVertex *vert);

float tlGetVertexNormalX(const TlVertex *vert);
float tlGetVertexNormalY(const TlVertex *vert);
float tlGetVertexNormalZ(const TlVertex *vert);

float tlGetVertexTexCoordS(const TlVertex *vert);
float tlGetVertexTexCoordT(const TlVertex *vert);

float tlGetVertexBinormalX(const TlVertex *vert);
float tlGetVertexBinormalY(const TlVertex *vert);
float tlGetVertexBinormalZ(const TlVertex *vert);

float tlGetVertexTangentX(const TlVertex *vert);
float tlGetVertexTangentY(const TlVertex *vert);
float tlGetVertexTangentZ(const TlVertex *vert);

TILE_EXTRNC_LEAVE

#endif

