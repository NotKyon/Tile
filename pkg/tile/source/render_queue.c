#include <tile/render_queue.h>
#include <tile/renderer.h>
#include <tile/opengl.h>
#include <tile/math.h>
#include <tile/entity.h>
#include <tile/view.h>
#include <tile/camera.h>
#include <tile/surface.h>
#include <tile/brush.h>

/*
 * ==========================================================================
 *
 *	RENDER QUEUE FUNCTIONS
 *
 * ==========================================================================
 */
static size_t g_numDrawItems = 0;
static size_t g_maxDrawItems = 0;
static TlDrawItem *g_drawItems = (TlDrawItem *)0;

static TlRenderQueueMode_t g_rqMode = kTlRQMode_StateSorted;

void tlRQ_SetMode(TlRenderQueueMode_t mode)
{
	g_rqMode = mode;
}
TlRenderQueueMode_t tlRQ_GetMode(void)
{
	return g_rqMode;
}

TlDrawItem *tlRQ_AddDrawItems(size_t numItems) {
#define DRAWITEM_GRAN 64
	size_t n;

	if (g_numDrawItems + numItems > g_maxDrawItems) {
		g_maxDrawItems  = g_numDrawItems + numItems;
		g_maxDrawItems -= g_maxDrawItems%DRAWITEM_GRAN;
		g_maxDrawItems += DRAWITEM_GRAN;

		n = g_maxDrawItems*sizeof(TlDrawItem);
		g_drawItems = (TlDrawItem *)tlMemory((void *)g_drawItems, n);
	}

	n = g_numDrawItems;
	g_numDrawItems += numItems;

	return &g_drawItems[n];
#undef DRAWITEM_GRAN
}
void tlRQ_AddEntities(TlEntity *ent, const struct TlMat4_s *V) {
	const TlMat4 *M;
	TlDrawItem *di;
	TlSurface *surf;
	TlEntity *chld;
	TlBrush *brush;
	size_t i, n;

	M = tlGetEntityGlobalMatrix(ent);
	tlAffineMultiply(&ent->MVP, V, M);

#if 0
	printf("ent->MVP = [ %3.3f, %3.3f, %3.3f, %3.3f,\n",
		ent->MVP[ 0], ent->MVP[ 4], ent->MVP[ 8], ent->MVP[12]);
	printf("             %3.3f, %3.3f, %3.3f, %3.3f,\n",
		ent->MVP[ 1], ent->MVP[ 5], ent->MVP[ 9], ent->MVP[13]);
	printf("             %3.3f, %3.3f, %3.3f, %3.3f,\n",
		ent->MVP[ 2], ent->MVP[ 6], ent->MVP[10], ent->MVP[14]);
	printf("             %3.3f, %3.3f, %3.3f, %3.3f ];\n\n",
		ent->MVP[ 3], ent->MVP[ 7], ent->MVP[11], ent->MVP[15]);
#endif

	for(surf=ent->s_head; surf!=(TlSurface *)0; surf=surf->s_next) {
		n = surf->numPasses;
		if( !n ) {
			continue;
		}

#if 0
		printf("Adding %i pass%s...\n", (int)n, n!=1 ? "es" : "");
#endif

		di = tlRQ_AddDrawItems(n);
		for(i=0; i<n; i++) {
			brush = surf->passes[i];

			di[i].order = i;
			di[i].M = &ent->MVP;
			di[i].brush = brush;
			di[i].surf = surf;
		}
	}

	for(chld=ent->head; chld!=(TlEntity *)0; chld=chld->next) {
		tlRQ_AddEntities(chld, V);
	}
}
int tlRQ_CmpFunc(const TlDrawItem *a, const TlDrawItem *b) {
	if( a->order != b->order ) {
		return a->order - b->order;
	}

	if( a->brush->drawing.zSort|b->brush->drawing.zSort ) {
		if( a->M != b->M ) {
			return a->M->zw > b->M->zw ? 1 : -1;
		}
	}

	if( a->brush->shader.prog != b->brush->shader.prog ) {
		return 1;
	}

	if( a->brush->drawing.zCmpFunc != b->brush->drawing.zCmpFunc ) {
		return 1;
	}

	if( a->surf != b->surf ) {
		return 1;
	}

	if( a->M != b->M ) {
		return a->M->zw < b->M->zw ? 1 : -1;
	}

	return 0;
}
void tlRQ_Sort() {
	qsort((void *)g_drawItems, g_numDrawItems, sizeof(TlDrawItem), (int(*)(const void *,const void *))tlRQ_CmpFunc);
}
size_t tlRQ_Count() {
	return g_numDrawItems;
}
size_t tlRQ_Capacity() {
	return g_maxDrawItems;
}
void tlRQ_Draw() {
	const void *inds;
	TlDrawItem *di;
	TlVertex *verts;
	size_t i;

	/*
	 * NOTE: This function is purposely NOT optimized
	 */

	tlGL_CheckError();

	glMatrixMode(GL_PROJECTION);
	tlGL_CheckError();

	glLoadMatrixf((const float *)tlGetViewMatrix(tlGetCameraEntity()->view));
	tlGL_CheckError();

	glMatrixMode(GL_MODELVIEW);
	tlGL_CheckError();
	glEnable(GL_DEPTH_TEST);
	tlGL_CheckError();

	for(i=0; i<g_numDrawItems; i++) {
		di = &g_drawItems[i];

		if( !di->brush->drawing.isVisible ) {
			continue;
		}

		verts = di->surf->verts;
		inds = (const void *)di->surf->inds;
		if( !verts || !inds ) {
			continue;
		}

		if( di->brush->lighting.isLit ) {
			glEnable(GL_LIGHTING);
		} else {
			glDisable(GL_LIGHTING);
		}
		tlGL_CheckError();

		switch(di->brush->drawing.cullMode) {
		case kTlCM_None:
			glDisable(GL_CULL_FACE);
			glCullFace(GL_NONE);
			break;
		case kTlCM_Front:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			break;
		case kTlCM_Back:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			break;
		case kTlCM_Both:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT_AND_BACK);
			break;
		default:
			break;
		}
		tlGL_CheckError();

		switch(di->brush->drawing.zCmpFunc) {
		case kTlCF_Never:			glDepthFunc(GL_NEVER);    break;
		case kTlCF_Less:			glDepthFunc(GL_LESS);     break;
		case kTlCF_LessEqual:		glDepthFunc(GL_LEQUAL);   break;
		case kTlCF_Equal:			glDepthFunc(GL_EQUAL);    break;
		case kTlCF_NotEqual:		glDepthFunc(GL_NOTEQUAL); break;
		case kTlCF_GreaterEqual:	glDepthFunc(GL_GEQUAL);   break;
		case kTlCF_Greater:		glDepthFunc(GL_GREATER);  break;
		case kTlCF_Always:		glDepthFunc(GL_ALWAYS);   break;
		default:
			break;
		}
		tlGL_CheckError();

		/*di->brush->drawing.zTest = TRUE;*/ /*HACK*/
		if( di->brush->drawing.zTest ) {
			glEnable(GL_DEPTH_TEST);
		} else {
			glDisable(GL_DEPTH_TEST);
		}
		tlGL_CheckError();

		glDepthMask(di->brush->drawing.zWrite);
		tlGL_CheckError();

		tlR_UseProgram(di->brush->shader.prog);
		tlGL_CheckError();

		glLoadMatrixf((const float *)di->M);
		tlGL_CheckError();

		glEnableClientState(GL_VERTEX_ARRAY);
		tlGL_CheckError();
		glEnableClientState(GL_COLOR_ARRAY);
		tlGL_CheckError();

		glVertexPointer(3, GL_FLOAT, sizeof(TlVertex), &verts->xyz);
		tlGL_CheckError();
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(TlVertex), &verts->color);
		tlGL_CheckError();
		glNormalPointer(GL_FLOAT, sizeof(TlVertex), &verts->norm);
		tlGL_CheckError();
		glTexCoordPointer(2, GL_FLOAT, sizeof(TlVertex), &verts->st);
		tlGL_CheckError();

		glDrawElements(GL_TRIANGLES, di->surf->numInds, GL_UNSIGNED_SHORT, inds);
		tlGL_CheckError();
	}

	g_numDrawItems = 0;
}

