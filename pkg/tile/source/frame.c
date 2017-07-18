#include <tile/frame.h>

static TlBool g_fg_didInit = FALSE;
static TlFrameGraph g_graph;
static TlResourceRegistry g_rcreg;

void tlFG_Init(void) {
	TL_ASSERT( g_fg_didInit == FALSE );

	memset( &g_graph, 0, sizeof(g_graph) );
	memset( &g_rcreg, 0, sizeof(g_rcreg) );

	g_fg_didInit = TRUE;
}
void tlFG_Fini(void) {
	if( !g_fg_didInit ) {
		return;
	}

	g_fg_didInit = FALSE;
}
TlBool tlFG_IsInitialized(void) {
	return g_fg_didInit;
}

#define TL_ASSERT_FG_INIT() TL_ASSERT( tlFG_IsInitialized() )

/*
===============================================================================

	FRAME GRAPH

===============================================================================
*/

TlFrameGraph *tlFG_FrameGraph(void) {
	TL_ASSERT_FG_INIT();
	return &g_graph;
}


/*
===============================================================================

	RESOURCE REGISTRY

===============================================================================
*/

TlResourceRegistry *tlFG_ResourceRegistry(void) {
	TL_ASSERT_FG_INIT();
	return &g_rcreg;
}


/*
===============================================================================

	RENDER PASS

===============================================================================
*/

static TlRenderPass *tlFG_AllocRenderPass( TlFrameGraph *graph ) {
	static const size_t gran = 16;

	TL_ASSERT( graph != ( TlFrameGraph * )0 );

	if( graph->renderPasses.num == graph->renderPasses.max ) {
		TlRenderPass *p;
		size_t newMax;

		newMax = graph->renderPasses.max + gran;

		p = tlReallocArrayZero( graph->renderPasses.ptr, graph->renderPasses.num, newMax, sizeof(TlRenderPass) );
		if( !p ) {
			TL_BREAKPOINT();
			tlErrorExit( "Insufficient memory for new RenderPass" );
		}

		graph->renderPasses.ptr = p;
		graph->renderPasses.max = newMax;
	}

	TL_ASSERT( graph->renderPasses.ptr != ( TlRenderPass * )0 );
	return &graph->renderPasses.ptr[ graph->renderPasses.num++ ];
}
static void tlFG_RemoveRenderPass( TlFrameGraph *graph, TlRenderPass *pass ) {
	size_t passIndex;

	TL_ASSERT( graph != ( TlFrameGraph * )0 );
	TL_ASSERT( pass != ( TlRenderPass * )0 );
	TL_ASSERT( graph->renderPasses.num > 0 );

	passIndex = ( size_t )( pass - graph->renderPasses.ptr );
	TL_ASSERT( passIndex < graph->renderPasses.num );

	if( passIndex + 1 < graph->renderPasses.num ) {
		memcpy( &graph->renderPasses.ptr[ passIndex ], &graph->renderPasses.ptr[ graph->renderPasses.num - 1 ], sizeof( TlRenderPass ) );
	}

	--graph->renderPasses.num;
}

TlRenderPass *tlFG_NewRenderPass( TlFnExecRenderPass pfnExec, void *data ) {
	TlRenderPass *pass;

	TL_ASSERT( pfnExec != ( TlFnExecRenderPass )0 );

	TL_ASSERT_FG_INIT();

	if( !( pass = tlFG_AllocRenderPass( tlFG_FrameGraph() ) ) ) {
		return ( TlRenderPass * )0;
	}

	pass->pfnExec = pfnExec;
	pass->data = data;

	return pass;
}
TlRenderPass *tlFG_DeleteRenderPass( TlRenderPass *pass ) {
	if( !pass ) {
		return ( TlRenderPass * )0;
	}

	TL_ASSERT_FG_INIT();

	tlFG_RemoveRenderPass( tlFG_FrameGraph(), pass );
	return ( TlRenderPass * )0;
}
