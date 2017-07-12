#include <tile/window.h>
#include <tile/view.h>
#include <tile/event.h>

#if !GLFW_ENABLED

/*                                                                            *
 * ========================================================================== *
 *   MS-WINDOWS                                                               *
 * ========================================================================== *
.*                                                                            */

# ifdef _WIN32

#  ifndef TILE_WINDOW_ICON_RESOURCE
#   define TILE_WINDOW_ICON_RESOURCE 1
#  endif
#  ifndef TILE_WINDOW_CURSOR_RESOURCE
#   define TILE_WINDOW_CURSOR_RESOURCE 2
#  endif

static TlWindow g_window;
static ATOM g_windowClass = ( ATOM )0;
static TlBool g_isRunning = FALSE;
static TlBool g_wasFullscreen = FALSE;

static TlU32 GetMods()
{
	TlU32 Mods = 0;

	Mods |= ( GetKeyState( VK_LSHIFT   ) < 0 ) ? kTlMF_LShift   : 0;
	Mods |= ( GetKeyState( VK_RSHIFT   ) < 0 ) ? kTlMF_RShift   : 0;
	Mods |= ( GetKeyState( VK_LMENU    ) < 0 ) ? kTlMF_LAlt     : 0;
	Mods |= ( GetKeyState( VK_RMENU    ) < 0 ) ? kTlMF_RAlt     : 0;
	Mods |= ( GetKeyState( VK_LCONTROL ) < 0 ) ? kTlMF_LControl : 0;
	Mods |= ( GetKeyState( VK_RCONTROL ) < 0 ) ? kTlMF_RControl : 0;
	Mods |= ( GetKeyState( VK_LBUTTON  ) < 0 ) ? kTlMF_Mouse1   : 0;
	Mods |= ( GetKeyState( VK_RBUTTON  ) < 0 ) ? kTlMF_Mouse2   : 0;
	Mods |= ( GetKeyState( VK_MBUTTON  ) < 0 ) ? kTlMF_Mouse3   : 0;
	Mods |= ( GetKeyState( VK_XBUTTON1 ) < 0 ) ? kTlMF_Mouse4   : 0;
	Mods |= ( GetKeyState( VK_XBUTTON2 ) < 0 ) ? kTlMF_Mouse5   : 0;

	return Mods;
}
static TlU32 GetScancode( LPARAM lParm )
{
	return ( ( lParm & 0x00FF0000 )>>16 ) + ( ( lParm & 0x01000000 )>>( 24 - 7 ) );
}
static TlBool IsRepeat( LPARAM lParm )
{
	return ( lParm & ( 1<<30 ) ) != 0 ? TRUE : FALSE;
}
static TlU32 ConvertUTF16ToUTF32( const TlU16 *src )
{
	// check if the value is encoded as one UTF16 word
	if( *src < 0xD800 || *src > 0xDFFF ) {
		return ( TlU32 )*src;
	}

	// check for an error (0xD800..0xDBFF)
	if( *src > 0xDBFF ) {
		// this is an invalid character; replace with U+FFFD
		return 0xFFFD;
	}

	// if no character follows, or is out of range, then this is an invalid encoding
	if( *( src + 1 ) < 0xDC00 || *( src + 1 ) > 0xDFFF ) {
		// replace with U+FFFD
		return 0xFFFD;
	}

	// encode
	return 0x10000 + ( ( ( TlU32 )( ( *src ) & 0x3FF ) << 10 ) | ( ( TlU32 )( *( src + 1 ) ) & 0x3FF ) );
}

static LRESULT CALLBACK tlWin_MsgProc_f( HWND hWnd, UINT uMsg, WPARAM wParm, LPARAM lParm )
{
	static TlU32 mouseCaptureCount = 0;

	switch( uMsg )
	{
	case WM_CLOSE:
		DestroyWindow( hWnd );
		break;
	case WM_DESTROY:
		PostQuitMessage( 0 );
		break;

	case WM_SIZE:
		GetClientRect( hWnd, &g_window.rcClient );
		tlRecalcAllViewports( g_window.rcClient.right, g_window.rcClient.bottom );
		break;

	case WM_ERASEBKGND:
		return 1;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
	case WM_MOUSEMOVE:
		{
			TlU32 mods;
			TlS32 clientX, clientY;

			mods = GetMods();
			clientX = ( TlS32 )( short )LOWORD( lParm );
			clientY = ( TlS32 )( short )HIWORD( lParm );

			if( uMsg == WM_MOUSEMOVE ) {
				TRACKMOUSEEVENT Tracker;

				Tracker.cbSize = sizeof( Tracker );
				Tracker.dwFlags = TME_LEAVE;
				Tracker.hwndTrack = hWnd;
				Tracker.dwHoverTime = 0;

				TrackMouseEvent( &Tracker );

				tlEv_EmitMouseMove( mods, clientX, clientY );
			} else {
				TlMouse_t mouse;

				mouse = kTlMouse1;
				if( uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP ) {
					mouse = kTlMouse2;
				} else if( uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONUP ) {
					mouse = kTlMouse3;
				} else if( uMsg == WM_XBUTTONDOWN || uMsg == WM_XBUTTONUP ) {
					mouse = ( TlMouse_t )( kTlMouse3 + ( TlU32 )HIWORD( wParm ) );
				}

				if( uMsg ==  WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN || uMsg == WM_MBUTTONDOWN || uMsg == WM_XBUTTONDOWN ) {
					tlEv_EmitMousePress( mouse, mods, clientX, clientY );
					SetCapture( hWnd );
					++mouseCaptureCount;
				} else {
					tlEv_EmitMouseRelease( mouse, mods, clientX, clientY );
					if( --mouseCaptureCount == 0 ) {
						ReleaseCapture();
					}
				}
			}
		}
		return 0;

	// mouse wheel
	case WM_MOUSEWHEEL:
		{
			TlU32 mods;
			TlS32 clientX, clientY;
			float delta;

			mods = GetMods();
			clientX = ( TlS32 )( short )LOWORD( lParm );
			clientY = ( TlS32 )( short )HIWORD( lParm );

			delta = ( float )( ( ( short )HIWORD( wParm ) )/WHEEL_DELTA );
			tlEv_EmitMouseWheel( mods, clientX, clientY, delta );
		}
		return 0;

	// mouse exit
	case WM_MOUSELEAVE:
		tlEv_EmitMouseExit( GetMods() );
		return 0;

	// key press
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		tlEv_EmitKeyPress( ( TlKey_t )GetScancode( lParm ), GetMods(), IsRepeat( lParm ) );
		return 0;

	// key release
	case WM_KEYUP:
	case WM_SYSKEYUP:
		tlEv_EmitKeyRelease( ( TlKey_t )GetScancode( lParm ), GetMods() );
		return 0;

	// key char
	case WM_CHAR:
		{
			TlU16 UTF16Char[ 2 ];

			UTF16Char[ 0 ] = ( TlU16 )( ( wParm >>  0 ) & 0xFFFF );
			UTF16Char[ 1 ] = ( TlU16 )( ( wParm >> 16 ) & 0xFFFF );

			tlEv_EmitKeyChar( ConvertUTF16ToUTF32( UTF16Char ) );
		}
		return 0;
	case WM_UNICHAR:
		if( wParm == UNICODE_NOCHAR ) {
			return TRUE;
		}

		tlEv_EmitKeyChar( ( TlU32 )wParm );
		return FALSE;

	default:
		break;
	}

	return DefWindowProcW( hWnd, uMsg, wParm, lParm );
}

TlWindow *tlWin_GetMain( void )
{
	return &g_window;
}

static TlBool tlWin_Register( void )
{
	WNDCLASSEXW wc;

	if( g_windowClass != ( ATOM )0 ) {
		return TRUE;
	}

	wc.cbSize = sizeof( wc );
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = &tlWin_MsgProc_f;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof( void * );
	wc.hInstance = GetModuleHandleW( NULL );
	wc.hIcon = ( HICON )LoadImageW( wc.hInstance, MAKEINTRESOURCEW( TILE_WINDOW_ICON_RESOURCE ), IMAGE_ICON, 32, 32, LR_CREATEDIBSECTION );
	wc.hCursor = ( HCURSOR )LoadImageW( wc.hInstance, MAKEINTRESOURCEW( TILE_WINDOW_CURSOR_RESOURCE ), IMAGE_CURSOR, 0, 0, LR_CREATEDIBSECTION );
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"TileWindow";
	wc.hIconSm = ( HICON )LoadImageW( wc.hInstance, MAKEINTRESOURCEW( TILE_WINDOW_ICON_RESOURCE ), IMAGE_ICON, 16, 16, LR_CREATEDIBSECTION );

	if( !wc.hCursor ) {
		wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	}

	g_windowClass = RegisterClassExW( &wc );
	if( !g_windowClass ) {
		tlErrorMessage( "Failed to register window class" );
		return FALSE;
	}

	memset( &g_window, 0, sizeof( g_window ) );
	return TRUE;
}

static DWORD tlWin_GetStyle( TlBool bIsFullscreen )
{
	return bIsFullscreen ? WS_POPUP : WS_OVERLAPPEDWINDOW;
}
static DWORD tlWin_GetExStyle( TlBool bIsFullscreen )
{
	return bIsFullscreen ? WS_EX_TOPMOST : 0;
}

static void tlWin_GLInit( void )
{
	PIXELFORMATDESCRIPTOR pfd;
	int pixfmt;

	if( g_window.hRC != NULL ) {
		return;
	}

	memset( &pfd, 0, sizeof( pfd ) );

	pfd.nSize = sizeof( pfd );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cAlphaBits = 8;
	pfd.cAccumBits = 0;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.cAuxBuffers = 0;
	pfd.iLayerType = PFD_MAIN_PLANE;

	g_window.hDC = GetDC( g_window.hWnd );

	pixfmt = ChoosePixelFormat( g_window.hDC, &pfd );
	if( !pixfmt ) {
		tlErrorMessage( "ChoosePixelFormat() failed" );
		exit( EXIT_FAILURE );
	}

	if( !SetPixelFormat( g_window.hDC, pixfmt, &pfd ) ) {
		tlErrorMessage( "SetPixelFormat() failed" );
		exit( EXIT_FAILURE );
	}

	g_window.hRC = wglCreateContext( g_window.hDC );
	if( !g_window.hRC ) {
		tlErrorMessage( "wglCreateContext() failed" );
		exit( EXIT_FAILURE );
	}

	wglMakeCurrent( g_window.hDC, g_window.hRC );
}

void tlWin_Init( unsigned int w, unsigned int h, TlBool fullscreen )
{
	if( !tlWin_Register() ) {
		exit( EXIT_FAILURE );
	}

	DWORD style;
	DWORD exstyle;
	RECT area;

	style = tlWin_GetStyle( fullscreen );
	exstyle = tlWin_GetExStyle( fullscreen );

	if( fullscreen ) {
		area.left = 0;
		area.top = 0;
		area.right = w;
		area.bottom = h;
	} else {
		area.left = GetSystemMetrics( SM_CXSCREEN )/2 - w/2;
		area.top = GetSystemMetrics( SM_CYSCREEN )/2 - h/2;
		area.right = area.left + w;
		area.bottom = area.top + h;
	}

	if( !AdjustWindowRectEx( &area, style, FALSE, exstyle ) ) {
		tlErrorMessage( "AdjustWindowRectEx failed" );
		exit( EXIT_FAILURE );
	}

	if( g_window.hWnd == NULL ) {
		g_window.hWnd = CreateWindowExW
		(
			exstyle,
			( LPCWSTR )( size_t )g_windowClass,
			L"",
			style,
			area.left,
			area.top,
			area.right - area.left,
			area.bottom - area.top,
			NULL,
			NULL,
			GetModuleHandleW( NULL ),
			NULL
		);
		if( !g_window.hWnd ) {
			tlErrorMessage( "CreateWindowExW failed" );
			exit( EXIT_FAILURE );
		}

		tlWin_GLInit();

		ShowWindow( g_window.hWnd, SW_SHOW );
		UpdateWindow( g_window.hWnd );
	} else {
		SetWindowLongPtrW( g_window.hWnd, GWL_EXSTYLE, exstyle );
		SetWindowLongPtrW( g_window.hWnd, GWL_STYLE, style );
		SetWindowPos( g_window.hWnd, HWND_TOP, area.left, area.top, area.right - area.left, area.bottom - area.top, SWP_FRAMECHANGED | SWP_SHOWWINDOW );
	}

	if( fullscreen ) {
		DEVMODE scrmode;
		LONG result;

		memset( &scrmode, 0, sizeof( scrmode ) );

		scrmode.dmSize = sizeof( DEVMODE );
		scrmode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
		scrmode.dmPelsWidth = w;
		scrmode.dmPelsHeight = h;

		result = ChangeDisplaySettings( &scrmode, CDS_FULLSCREEN );
		if( result != DISP_CHANGE_SUCCESSFUL ) {
			tlErrorMessage( "Failed to change display settings" );
			exit( EXIT_FAILURE );
		}

		g_wasFullscreen = TRUE;
	} else {
		if( g_wasFullscreen ) {
			ChangeDisplaySettings( NULL, 0 );
		}

		g_wasFullscreen = FALSE;
	}

	g_isRunning = TRUE;
}
void tlWin_Fini( void )
{
	g_isRunning = FALSE;

	if( !g_window.hWnd ) {
		return;
	}

	if( g_window.hRC != NULL ) {
		wglMakeCurrent( NULL, NULL );
		wglDeleteContext( g_window.hRC );
		g_window.hRC = NULL;
	}

	if( IsWindow( g_window.hWnd ) ) {
		HWND hWnd;

		hWnd = g_window.hWnd;
		g_window.hWnd = NULL;

		if( g_window.hDC != NULL ) {
			ReleaseDC( hWnd, g_window.hDC );
			g_window.hDC = NULL;
		}

		DestroyWindow( hWnd );
	}

	if( g_wasFullscreen ) {
		ChangeDisplaySettings( NULL, 0 );
		g_wasFullscreen = FALSE;
	}
}

TlBool tlWin_IsOpen( void )
{
	return g_isRunning;
}
TlBool tlWin_Loop( void )
{
	MSG Msg;

	while( PeekMessage( &Msg, NULL, 0, 0, PM_REMOVE ) ) {
		if( Msg.message == WM_QUIT ) {
			g_isRunning = FALSE;
		}

		TranslateMessage( &Msg );
		DispatchMessageW( &Msg );
	}

	return g_isRunning;
}

void tlWin_SwapBuffers( void )
{
	SwapBuffers( g_window.hDC );
}

TlU32 tlWin_ResX( void )
{
	return g_window.rcClient.right;
}
TlU32 tlWin_ResY( void )
{
	return g_window.rcClient.bottom;
}
# endif

#endif
