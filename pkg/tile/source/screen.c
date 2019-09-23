#include <tile/screen.h>
#include <tile/opengl.h>
#include <tile/view.h>
#include <tile/window.h>

#if GLFW_ENABLED
# include <tile/event.h>
#else
extern void tlEv_Init( void );
#endif

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

static void glfw_onClosed_f(GLFWwindow *wnd) {
	glfwSetWindowShouldClose( wnd, GLFW_TRUE );
	
	if( tl__g_window == wnd ) {
		tl__g_window = (GLFWwindow*)0;
	}
}

static TlU32 glfw_cvtmods( int mods )
{
	TlU32 cvtmods = 0;

	if( mods & GLFW_MOD_SHIFT ) {
		cvtmods |= kTlMF_LShift;
	}
	if( mods & GLFW_MOD_CONTROL ) {
		cvtmods |= kTlMF_LControl;
	}
	if( mods & GLFW_MOD_ALT ) {
		cvtmods |= kTlMF_LAlt;
	}

	return cvtmods;
}
static void glfw_getMouse( GLFWwindow *window, TlS32 *dstClientPosX, TlS32 *dstClientPosY )
{
	double mousePosX, mousePosY;

	glfwGetCursorPos( window, &mousePosX, &mousePosY );

	*dstClientPosX = (TlS32)( mousePosX );
	*dstClientPosY = (TlS32)( mousePosY );
}
static TlMouse_t glfw_getMouseButton( int button )
{
	// GLFW_MOUSE_BUTTON_1 = 0, GLFW_MOUSE_BUTTON_8 = 7
	// EMouse::Button1 = 1, EMouse::Button8 = 8

	return (TlMouse_t)( button + 1 );
}

static void glfw_windowFocus_f( GLFWwindow *window, int focused )
{
	/*
	if( focused ) {
		(void)in_onAcceptKey_f( OSWindow(0) );
	} else {
		(void)in_onResignKey_f( OSWindow(0) );
	}
	*/
	((void)window);
	((void)focused);
}

static void glfw_mouseButton_f( GLFWwindow *window, int inbutton, int action, int inmods )
{
	TlS32 clientPosX, clientPosY;
	glfw_getMouse( window, &clientPosX, &clientPosY );

	const TlMouse_t button = glfw_getMouseButton( inbutton );
	const TlU32 mods = glfw_cvtmods( inmods );

	if( action ) {
		tlEv_EmitMousePress( button, mods, clientPosX, clientPosY );
	} else {
		tlEv_EmitMouseRelease( button, mods, clientPosX, clientPosY );
	}
}
static void glfw_mousePos_f( GLFWwindow *window, double x, double y )
{
	const TlS32 clientPosX = (TlS32)( x );
	const TlS32 clientPosY = (TlS32)( y );

	// FIXME: Use current mods
	const TlU32 mods = 0;

	((void)window);

	tlEv_EmitMouseMove( mods, clientPosX, clientPosY );
}
static void glfw_mouseEnterLeave_f( GLFWwindow *window, int entered )
{
	((void)window);

	if( !entered ) {
		// FIXME: Use current mods
		const TlU32 mods = 0;

		tlEv_EmitMouseExit( mods );
	}
}
static void glfw_mouseScroll_f( GLFWwindow *window, double x, double y )
{
	static const double epsilon = 1e-6;

	((void)x);

	if( y >= -epsilon && y <= epsilon ) {
		return;
	}

	const float delta = (float)( y );

	TlS32 clientPosX, clientPosY;
	glfw_getMouse( window, &clientPosX, &clientPosY );

	// FIXME: Use current mods
	const TlU32 mods = 0;

	tlEv_EmitMouseWheel( mods, clientPosX, clientPosY, delta );
}

static TlKey_t glfw_keyFromScancode( int scancode )
{
#if defined(__linux__)
	return (TlKey_t)( scancode - 1 ); // X11 just maps to +1
#elif defined(__APPLE__)
	//axpf( "scancode -> %i\n", scancode );
	// http://boredzo.org/blog/wp-content/uploads/2007/05/IMTx-virtual-keycodes.pdf
	switch( scancode ) {
# define MAP(MacOSKeyCode_,DollKeyName_) case MacOSKeyCode_: return kTlKey_##DollKeyName_
	MAP( 53, Escape ); MAP( 50, Grave );
	MAP( 18, One ); MAP( 19, Two ); MAP( 20, Three ); MAP( 21, Four );
	MAP( 23, Five ); MAP( 22, Six ); MAP( 26, Seven ); MAP( 28, Eight );
	MAP( 25, Nine ); MAP( 29, Zero );
	MAP( 27, Minus );
	MAP( 24, Equals );
	MAP( 51, Delete );
	MAP( 48, Tab );

	MAP( 12, Q ); MAP( 13, W ); MAP( 14, E ); MAP( 15, R );
	MAP( 17, T ); MAP( 16, Y ); MAP( 32, U ); MAP( 34, I );
	MAP( 31, O ); MAP( 35, P ); MAP( 33, LBracket ); MAP( 30, RBracket );
	MAP( 42, Backslash );

	MAP( 57, CapsLock );
	MAP( 0, A ); MAP( 1, S ); MAP( 2, D ); MAP( 3, F );
	MAP( 5, G ); MAP( 4, H ); MAP( 38, J ); MAP( 40, K );
	MAP( 37, L ); MAP( 41, Semicolon ); MAP( 39, Apostrophe ); MAP( 36, Return );

	MAP( 56, LShift );
	MAP( 6, G ); MAP( 7, X ); MAP( 8, C ); MAP( 9, V );
	MAP( 11, B ); MAP( 45, N ); MAP( 46, M ); MAP( 43, Comma );
	MAP( 47, Period ); MAP( 44, Slash );

	MAP( 59, LControl );
	MAP( 58, LAlt );
	MAP( 55, LSuper );
	MAP( 49, Space );

	MAP( 123, Left ); MAP( 125, Down ); MAP( 124, Right ); MAP( 126, Up );

	MAP( 114, Insert ); MAP( 115, Home ); MAP( 116, Prior );
	MAP( 117, Delete ); MAP( 119, End ); MAP( 121, Next );

	MAP( 122, F1 ); MAP( 120, F2 ); MAP( 99, F3 ); MAP( 118, F4 );
	MAP( 96, F5 ); MAP( 97, F6 ); MAP( 98, F7 ); MAP( 100, F8 );
	MAP( 101, F9 ); MAP( 109, F10 ); MAP( 103, F11 ); MAP( 111, F12 );
	MAP( 105, F13 ); MAP( 107, F14 ); MAP( 113, F15 );

	MAP( 71, NumLock );
	MAP( 81, NumPadEquals );
	MAP( 75, Divide );
	MAP( 67, Multiply );
	MAP( 78, Subtract );
	MAP( 69, Add );
	MAP( 76, NumPadEnter );
	MAP( 65, Decimal );

	MAP( 89, NumPad7 ); MAP( 91, NumPad8 ); MAP( 92, NumPad9 );
	MAP( 86, NumPad4 ); MAP( 87, NumPad5 ); MAP( 88, NumPad6 );
	MAP( 83, NumPad1 ); MAP( 84, NumPad2 ); MAP( 85, NumPad3 );
	MAP( 82, NumPad0 );
# undef MAP

	default:
		break;
	}

	return kTlKey_None;
#else
	return (TlKey_t)( scancode );
#endif
}
static void glfw_keyButton_f( GLFWwindow *window, int _1, int scancode, int action, int inmods )
{
	((void)window);
	((void)_1);

	const TlKey_t key = glfw_keyFromScancode( scancode );
	const TlU32 mods = glfw_cvtmods( inmods );
	const TlBool isRepeat = ( action == GLFW_REPEAT );

	if( action ) {
		tlEv_EmitKeyPress( key, mods, isRepeat );
	} else {
		tlEv_EmitKeyRelease( key, mods );
	}
}
static void glfw_keyChar_f( GLFWwindow *window, unsigned int codepoint )
{
	((void)window);

	tlEv_EmitKeyChar( codepoint );
}
#endif

void tlScr_Init(TlScreen *desc) {
#define APPLYDEFAULT(x,y) if(!(x))x=y
#define ASPECT(x,y) ((int)(((double)(x))/((double)(y))))
	TlScreen internalScreenDesc;
	GLFWwindow *w;

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
	glfwWindowHint( GLFW_RESIZABLE, GLFW_TRUE );

# ifdef __APPLE__
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 2 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
# endif
	w = glfwCreateWindow( desc->width, desc->height, "Tile", ( GLFWmonitor * )0, ( GLFWwindow * )0 );
	if( !w ) {
		fprintf( stderr, "Error: Failed to create OpenGL window.\n" );
		exit( EXIT_FAILURE );
	}

	tl__g_window = w;
	
	/*glfwSetWindowSizeCallback( tl__g_window, &ReshapeWindow );*/

	glfwSetWindowCloseCallback( w, &glfw_onClosed_f );
	glfwSetFramebufferSizeCallback( w, &ReshapeWindow );

	glfwSetWindowFocusCallback( w, &glfw_windowFocus_f );

	glfwSetMouseButtonCallback( w, &glfw_mouseButton_f );
	glfwSetCursorPosCallback( w, &glfw_mousePos_f );
	glfwSetCursorEnterCallback( w, &glfw_mouseEnterLeave_f );
	glfwSetScrollCallback( w, &glfw_mouseScroll_f );

	glfwSetKeyCallback( w, &glfw_keyButton_f );
	glfwSetCharCallback( w, &glfw_keyChar_f );

	glfwMakeContextCurrent( w );

	glfwShowWindow( tl__g_window );
	glfwFocusWindow( tl__g_window );
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
	if( tl__g_window != (GLFWwindow*)0 ) {
		glfwDestroyWindow(tl__g_window);
		tl__g_window = (GLFWwindow*)0;
	}
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

