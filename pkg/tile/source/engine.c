#include <tile/engine.h>
#include <tile/screen.h>
#include <tile/renderer.h>
#include <tile/camera.h>
#include <tile/system.h>

static TlBool g_isTimingCurrent = FALSE;
static TlU64 g_currTime = 0;
static TlU64 g_prevTime = 0;
static double g_deltaTime = 0.0;

TlBool tlInit(void)
{
	tlScr_Init((TlScreen *)0);
	tlR_Init();

	tlSetCameraAutoAspect(1280.0/720.0, kTlAspect_Fit);
	
	g_isTimingCurrent = FALSE;
	g_currTime = tlSys_Microtime();
	g_prevTime = g_currTime;
	g_deltaTime = 0.0;
	
	return TRUE;
}
void tlFini(void)
{
	tlR_Fini();
	tlScr_Fini();
}

static void tlUpdateTiming(void)
{
	TlU64 currTime;
	
	currTime = tlSys_Microtime();
	if( g_isTimingCurrent ) {
		g_prevTime = g_currTime;
	} else {
		g_prevTime = currTime;
		g_isTimingCurrent = TRUE;
	}
	g_currTime = currTime;
	
	g_deltaTime = ((double)(g_currTime - g_prevTime))/1000000.0;
}

TlBool tlLoop(void)
{
	tlR_Frame( tlGetDeltaTime() );
	if( !tlScr_IsOpen() ) {
		return FALSE;
	}

	tlUpdateTiming();

	return TRUE;
}

double tlGetDeltaTime(void)
{
	return g_deltaTime;
}
