#ifndef TILE_WINDOW_H
#define TILE_WINDOW_H

#include "const.h"

TILE_EXTRNC_ENTER

#if !GLFW_ENABLED

# ifdef _WIN32
typedef struct TlWindow_s
{
	HWND		hWnd;
	HDC			hDC;
	HGLRC		hRC;

	RECT		rcClient;
} TlWindow;

TlWindow *tlWin_GetMain( void );

void tlWin_Init( unsigned int w, unsigned int h, TlBool fullscreen );
void tlWin_Fini( void );

TlBool tlWin_IsOpen( void );
TlBool tlWin_Loop( void );

void tlWin_SwapBuffers( void );

TlU32 tlWin_ResX( void );
TlU32 tlWin_ResY( void );
# endif

#endif

TILE_EXTRNC_LEAVE

#endif
