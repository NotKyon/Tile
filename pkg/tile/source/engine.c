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

static void tlMicroSleep( TlU64 microseconds )
{
#ifdef _WIN32
	const TlU32 milliseconds = (TlU32)( ( microseconds + 500 )/1000 );
	if( milliseconds > 0 ) {
		Sleep( milliseconds );
	}
#else
	struct timespec ts;
	ts.tv_sec = microseconds/1000000;
	ts.tv_nsec = ( microseconds%1000000 )*1000;
	while( nanosleep( &ts, &ts ) == -1 && errno == EINTR ) {
		((void)0);
	}
#endif
}

TlBool tlLoop(void)
{
	static const TlU64 timeBudgetMicrosec = 16666; /* FIXME: Make configurable */
	TlU64 deltaMicrosec;

	tlR_Frame( tlGetDeltaTime() );
	if( !tlScr_IsOpen() ) {
		return FALSE;
	}

	deltaMicrosec = tlSys_Microtime() - g_currTime;
	if( deltaMicrosec < timeBudgetMicrosec ) {
		tlMicroSleep( deltaMicrosec );
	}

	tlUpdateTiming();

	return TRUE;
}

double tlGetDeltaTime(void)
{
	return g_deltaTime;
}
