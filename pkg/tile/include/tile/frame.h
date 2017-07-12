#ifndef TILE_FRAME_H
#define TILE_FRAME_H

/*

	+-----------+
	| <Present> |
	+-----------+

*/

#include "const.h"
#include "math.h"
#include "renderer.h"

TILE_EXTRNC_ENTER

struct TlFG_Resource_s;

struct TL_CACHELINE_ALIGNED TlFrameGraph_s;
struct TL_CACHELINE_ALIGNED TlRenderPass_s;
struct TL_CACHELINE_ALIGNED TlResourceRegistry_s;

typedef struct TlFrameGraph_s TlFrameGraph;
typedef struct TlRenderPass_s TlRenderPass;
typedef struct TlResourceRegistry_s TlResourceRegistry;

typedef union { TlU32 index; } TlResource;
typedef union { TlU32 index; } TlMutableResource;

typedef enum {
	kTlRcTy_None,

	kTlRcTy_Buffer,
	kTlRcTy_Texture
} TlResourceType_t;

typedef enum {
	kTlRFmt_None,

	kTlRFmt_RGBA8UN,
	kTlRFmt_RGB10A2UN,
	kTlRFmt_RGBA16F,
	kTlRFmt_RGBA32F,
	kTlRFmt_R32F,
	kTlRFmt_D32F,
	kTlRFmt_D24F_S8UI,
} TlRenderFormat_t;

typedef enum {
	kTlTexInitState_Clear,
	kTlTexInitState_Discard
} TlTextureInitState_t;

typedef struct TlFG_TextureDesc_s {
	/* Width of the texture (resolution along x-axis) */
	TlU32 resX;
	/* Height of the texture (resolution along y-axis) */
	TlU32 resY;

	/* Format to store texels in */
	TlRenderFormat_t format;
	/* Initial state of the texture data */
	TlTextureInitState_t initState;
} TlFG_TextureDesc_t;

/*
 * ----------
 * FrameGraph
 * ----------
 * Used to construct the rendering operations
 */
struct TlFrameGraph_s {
	/* Flat array of render passes */
	struct {
		TlRenderPass_s *ptr;
		size_t          num;
		size_t          max;
	} renderPasses;

	/* Compilation stack for culling unused resources */
	struct TlFG_Resource_s *compileStack_head;
};

void tlFG_Init(void);
void tlFG_Fini(void);

TlFrameGraph *tlFG_FrameGraph(void);

/*
 * ----------------
 * ResourceRegistry
 * ----------------
 * Manages general resources
 *
 * NOTE: TlResource/TlMutableResource's `index` field indexes the `resources`
 *       array in here.
 */
struct TlResourceRegistry_s {
	/* Flat array of render resources */
	struct {
		struct TlFG_Resource_s *ptr;
		size_t                  num;
		size_t                  max;
	} resources;
};

TlResourceRegistry *tlFG_ResourceRegistry(void);

/*
 * ----------
 * RenderPass
 * ----------
 * Defines several phases for rendering
 */
typedef void(*TlFnExecRenderPass)(TlRenderPass*,void*);
struct TlRenderPass_s {
	/* Reference count for every resource write */
	TlU32 refCount;

	/* Resources used as inputs */
	struct {
		TlResource *ptr;
		size_t      num;
	} reads;
	/* Resources used as outputs */
	struct {
		TlMutableResource *ptr;
		size_t             num;
	} writes;

	/* Execution function */
	TlFnExecRenderPass pfnExec;

	/* Additional data to pass to these callbacks */
	void *data;
};

TlRenderPass *tlFG_NewRenderPass( TlFnExecRenderPass pfnExec, void *data );
TlRenderPass *tlFG_DeleteRenderPass( TlRenderPass *pass );

TlResource tlFG_RenderPass_Read( TlRenderPass *pass, TlResource input );
TlMutableResource tlFG_RenderPass_Write( TlRenderPass *pass, TlMutableResource output, TlU32 flags );
TlMutableResource tlFG_RenderPass_UseRenderTarget( TlRenderPass *pass, TlU32 index );
TlMutableResource tlFG_RenderPass_UseDepthStencil( TlRenderPass *pass );
TlMutableResource tlFG_RenderPass_CreateTexture( TlRenderPass *pass, TlFG_TextureDesc_t *createInfo );

/*
 * --------------
 * RenderResource
 * --------------
 * Internal structure for resources
 */

struct TlFG_Resource_s {
	/* Reference count for every used read */
	TlU32 refCount;

	/* The render pass that created us (or NULL) */
	TlRenderPass *producer;

	/* Pointer to the next resource in the compilation stack */
	struct TlFG_Resource_s *compileStack_next;
};

TILE_EXTRNC_LEAVE

#endif
