#include <tile/screen.h>
#include <tile/opengl.h>
#include <tile/view.h>
#include <tile/window.h>

extern void tlEv_Init( void );

#if GLFW_ENABLED
GLFWwindow *tl__g_window = ( GLFWwindow * )0;
#endif

/*
 * ==========================================================================
 *
 *	SCREEN
 *
 * ==========================================================================
 */
#if GLFW_ENABLED
void GLFWAPI ReshapeWindow(GLFWwindow *wnd, int w, int h) {
	((void)wnd);
	tlRecalcAllViewports(w, h);
}
#endif

void tlScr_Init(TlScreen *desc) {
#define APPLYDEFAULT(x,y) if(!(x))x=y
#define ASPECT(x,y) ((int)(((double)(x))/((double)(y))))
	TlScreen internalScreenDesc;

	tlEv_Init();

#if GLFW_ENABLED
	glfwInit();
	atexit(glfwTerminate);
#endif

	if (!desc) {
		memset(&internalScreenDesc, 0, sizeof(internalScreenDesc));
		desc = &internalScreenDesc;
	}

	APPLYDEFAULT(desc->width  , 1024);
	APPLYDEFAULT(desc->height , ASPECT(desc->width, 16.0/9.0));
	APPLYDEFAULT(desc->r      , 8);
	APPLYDEFAULT(desc->g      , 8);
	APPLYDEFAULT(desc->b      , 8);
	APPLYDEFAULT(desc->a      , 8);
	APPLYDEFAULT(desc->depth  , 24);
	APPLYDEFAULT(desc->stencil, 8);

#if GLFW_ENABLED
	/*
	if( !glfwOpenWindow(desc->width, desc->height, desc->r, desc->g, desc->b, desc->a, desc->depth, desc->stencil, GLFW_WINDOW) ) {
		exit(EXIT_FAILURE);
	}
	*/
	tl__g_window = glfwCreateWindow( desc->width, desc->height, "Tile", ( GLFWmonitor * )0, ( GLFWwindow * )0 );
	if( !tl__g_window ) {
		fprintf( stderr, "Error: Failed to create OpenGL window.\n" );
		exit( EXIT_FAILURE );
	}
	glfwSetWindowSizeCallback( tl__g_window, &ReshapeWindow );

	/*printf("GLFW_ACCELERATED=%i\n", glfwGetWindowParam(GLFW_ACCELERATED));
	printf("GLFW_RED_BITS=%i\n", glfwGetWindowParam(GLFW_RED_BITS));
	printf("GLFW_GREEN_BITS=%i\n", glfwGetWindowParam(GLFW_GREEN_BITS));
	printf("GLFW_BLUE_BITS=%i\n", glfwGetWindowParam(GLFW_BLUE_BITS));
	printf("GLFW_ALPHA_BITS=%i\n", glfwGetWindowParam(GLFW_ALPHA_BITS));
	printf("GLFW_DEPTH_BITS=%i\n", glfwGetWindowParam(GLFW_DEPTH_BITS));
	printf("GLFW_STENCIL_BITS=%i\n", glfwGetWindowParam(GLFW_STENCIL_BITS));
	printf("\n");*/
#elif defined( _WIN32 )
	tlWin_Init( desc->width, desc->height, FALSE );
	atexit( &tlWin_Fini );
#endif

	printf("GL Version: %s\n", (const char *)glGetString(GL_VERSION));
	printf("GL Renderer: %s\n", (const char *)glGetString(GL_RENDERER));
	printf("GL Vendor: %s\n", (const char *)glGetString(GL_VENDOR));
	printf("GL Extensions: %s\n", (const char *)glGetString(GL_EXTENSIONS));
	printf("\n");

	atexit(&tlScr_Fini);

	tlGL_RequireExtension("GL_ARB_shader_objects");
	tlGL_RequireExtension("GL_ARB_shading_language_100");
#undef ASPECT
#undef APPLYDEFAULT
}
void tlScr_Fini() {
#if GLFW_ENABLED
	glfwDestroyWindow(tl__g_window);
#elif defined( _WIN32 )
	tlWin_Fini();
#endif
}
TlBool tlScr_IsOpen() {
#if GLFW_ENABLED
	return tl__g_window != ( GLFWwindow * )0 &&
		!glfwWindowShouldClose(tl__g_window);
#elif defined( _WIN32 )
	return tlWin_IsOpen();
#endif
}

