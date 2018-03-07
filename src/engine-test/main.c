#include <tile.h>

/* provided by mk */
#include <build-generated/gitinfo.h>

/*

	TODO
	- Fix various input handling systems (they were written quickly, without thought)
	- Fix mouse handling (feels "clunky")
	- Fix wheel scrolling (probably MoveZ math error)

*/

/*
===============================================================================

	TRI-CONTROL CAMERA SYSTEM

	Controls:
	- Hold left mouse button to look around (FPS-style)
	- Hold right mouse button to pan camera
	- Hold middle mouse button (or control + left mouse button) to orbit
	  around a pivot point

===============================================================================
*/

typedef enum
{
	kCamLock_None,

	kCamLock_Look,
	kCamLock_Pan,
	kCamLock_Orbit
} CamLock_t;

CamLock_t g_cam_lock = kCamLock_None;
TlMouse_t g_cam_releaselock = kTlMouse_None;
TlVec3 g_cam_pivot = { 0, 0, 0 };
int g_cam_mouselock[ 2 ] = { 0, 0 };
double g_cam_look[ 2 ] = { 0.0f, 0.0f };
TlVec3 g_cam_move = { 0, 0, 0 };
TlVec3 g_cam_turn = { 0, 0, 0 };
double g_cam_dist = 0.0;

/*
----------------
cam_onEvent

Returns TRUE if the event was handled by this system and no further processing
is necessary. Otherwise returns FALSE, indicating the event can be handled by a
different system.
----------------
*/
TlBool cam_onEvent( void )
{
	const TlEvent *ev;

	ev = tlEv_Current();
	switch( g_cam_lock ) {
	case kCamLock_None:
		{
			if( ev->type == kTlEv_MousePress ) {
				if( ev->mouse == kTlMouse1 && tlControlKey() ) {
					g_cam_lock = kCamLock_Orbit;
					g_cam_releaselock = kTlMouse1;
				} else if( ev->mouse == kTlMouse1 ) {
					g_cam_lock = kCamLock_Look;
					g_cam_releaselock = kTlMouse1;
				} else if( ev->mouse == kTlMouse2 ) {
					g_cam_lock = kCamLock_Pan;
					g_cam_releaselock = kTlMouse2;
				} else if( ev->mouse == kTlMouse3 ) {
					g_cam_lock = kCamLock_Orbit;
					g_cam_releaselock = kTlMouse3;
				}

				if( g_cam_lock != kCamLock_None ) {
					const TlVec3 *diff;
					
					g_cam_mouselock[ 0 ] = tlMouseX();
					g_cam_mouselock[ 1 ] = tlMouseY();

					g_cam_look[ 0 ] = 0.0f;
					g_cam_look[ 1 ] = 0.0f;

					diff = tlSubVec3( tlGetEntityPosition( tlGetCameraEntity() ), &g_cam_pivot );
					g_cam_dist = tlSqrt( tlDot( diff, diff ) );

					return TRUE;
				}
			}
		}
		break;

	default:
		{
			if( ev->type == kTlEv_MouseMove ) {
				return TRUE;
			}

			if( ev->type == kTlEv_MouseRelease ) {
				if( ev->mouse == g_cam_releaselock ) {
					g_cam_lock = kCamLock_None;
					g_cam_releaselock = kTlMouse_None;
					return TRUE;
				}
			}
		}
		break;
	}

	if( ev->type == kTlEv_KeyPress || ev->type == kTlEv_KeyRelease ) {
		if( ev->key == kTlKey_W || ev->key == kTlKey_A || ev->key == kTlKey_S || ev->key == kTlKey_D ) {
			return TRUE;
		}

		if( ev->key == kTlKey_Left || ev->key == kTlKey_Up || ev->key == kTlKey_Right || ev->key == kTlKey_Down ) {
			return TRUE;
		}
	}
	if( ev->type == kTlEv_KeyChar && !tlControlKey() ) {
		if( ev->utf32Char == ( TlU32 )+'w' || ev->utf32Char == ( TlU32 )+'W' ) {
			return TRUE;
		}
		if( ev->utf32Char == ( TlU32 )+'a' || ev->utf32Char == ( TlU32 )+'A' ) {
			return TRUE;
		}
		if( ev->utf32Char == ( TlU32 )+'s' || ev->utf32Char == ( TlU32 )+'S' ) {
			return TRUE;
		}
		if( ev->utf32Char == ( TlU32 )+'d' || ev->utf32Char == ( TlU32 )+'D' ) {
			return TRUE;
		}
	}

	if( ev->type == kTlEv_MouseWheel ) {
		return TRUE;
	}

	return FALSE;
}

/*
----------------
cam_onFrame

Updates the camera
----------------
*/
void cam_onFrame( double deltaTime )
{
	double camspd = 0.01f, camtrn = 0.25f;
	double keyspd = 1.0f, keytrn = 36.0f;
	double distance;

#if 0
	g_cam_look[ 0 ] = ( double )( 840*( tlMouseX() - g_cam_mouselock[ 0 ] ) );
	g_cam_look[ 1 ] = ( double )( 840*( tlMouseY() - g_cam_mouselock[ 1 ] ) );
#else
	g_cam_look[ 0 ] = ( double )( 840*tlMouseMoveX() );
	g_cam_look[ 1 ] = ( double )( 840*tlMouseMoveY() );
#endif

	g_cam_move.z += tlMouseWheel()*123.0f;
	tlClearMouseWheel();

#ifdef _XXXWIN32
	if( g_cam_lock != kCamLock_None ) {
		POINT pt;

		pt.x = g_cam_mouselock[ 0 ];
		pt.y = g_cam_mouselock[ 1 ];

		ClientToScreen( GetActiveWindow(), &pt );
		SetCursorPos( pt.x, pt.y );

		tlClearMouseMove();
	}
#else
	g_cam_mouselock[ 0 ] = tlMouseX();
	g_cam_mouselock[ 1 ] = tlMouseY();
#endif

	if( tlKeyState( kTlKey_W ) || tlKeyState( kTlKey_Up ) ) {
		g_cam_move.z += keyspd;
	}
	if( tlKeyState( kTlKey_S ) || tlKeyState( kTlKey_Down ) ) {
		g_cam_move.z -= keyspd;
	}

	if( tlKeyState( kTlKey_A ) ) {
		g_cam_move.x -= keyspd;
	}
	if( tlKeyState( kTlKey_D ) ) {
		g_cam_move.x += keyspd;
	}

	if( tlKeyState( kTlKey_Left ) ) {
		g_cam_turn.y -= keytrn;
	}
	if( tlKeyState( kTlKey_Right ) ) {
		g_cam_turn.y += keytrn;
	}

	distance = 0.0f;

	switch( g_cam_lock ) {
	case kCamLock_None:
		break;

	case kCamLock_Look:
		g_cam_turn.x = ( float )( g_cam_look[ 1 ]*camtrn );
		g_cam_turn.y = ( float )( g_cam_look[ 0 ]*camtrn );
		break;

	case kCamLock_Pan:
		g_cam_move.x += ( float )( g_cam_look[ 0 ]*camspd );
		g_cam_move.y -= ( float )( g_cam_look[ 1 ]*camspd );
		break;

	case kCamLock_Orbit:
		{
			tlSetCameraPosition( g_cam_pivot.x, g_cam_pivot.y, g_cam_pivot.z );

			g_cam_turn.x = ( float )( g_cam_look[ 1 ]*camtrn );
			g_cam_turn.y = ( float )( g_cam_look[ 0 ]*camtrn );
		}
		break;
	}

	if( g_cam_turn.x != 0.0f || g_cam_turn.y != 0.0f ) {
		tlTurnCameraY( g_cam_turn.y*deltaTime );
		tlTurnCameraX( g_cam_turn.x*deltaTime );
	}

	if( g_cam_lock == kCamLock_Orbit ) {
		g_cam_dist -= g_cam_move.z;
		g_cam_move.z = 0.0f;
		
		if( g_cam_dist < 0.0125 ) {
			g_cam_dist = 0.0125;
		}

		tlMoveCameraZ( ( float )-g_cam_dist );
	}

	if( g_cam_move.x != 0.0f || g_cam_move.y != 0.0f || g_cam_move.z != 0.0f ) {
		if( g_cam_lock != kCamLock_Orbit ) {
			tlMoveCamera( g_cam_move.x*deltaTime, g_cam_move.y*deltaTime, g_cam_move.z*deltaTime );
		}
	}

	g_cam_look[ 0 ] = 0.0f;
	g_cam_look[ 1 ] = 0.0f;
	g_cam_turn.x = 0.0f;
	g_cam_turn.y = 0.0f;
	g_cam_move.x = 0.0f;
	g_cam_move.y = 0.0f;
	g_cam_move.z = 0.0f;
}


/*
===============================================================================

	MAIN TESTING PROGRAM

===============================================================================
*/

TlEntity *g_shape = (TlEntity *)0;
TlEntity *g_cube = (TlEntity *)0;
TlEntity *g_child = (TlEntity *)0;
TlEntity *g_plane = (TlEntity *)0;

TlBool update(double deltaTime) {
	double speed;

	/* hacked in processing for frame animation */
	#if 0
#if 1
	tlTurnEntityX(g_shape, 45.0f*deltaTime);
	tlTurnEntityY(g_shape, -45.0f*deltaTime);
	/*tlTurnEntityZ(g_shape, 45.0f*time);*/
#endif

#if 1
	tlSetEntityPosition(g_shape, 0,0, 1 + (1 + tlSin(90.0f*time))/2*4);
#else
	tlSetEntityPosition(g_shape, 0, 0, 1);
#endif

	tlTurnEntityY(g_cube, 12.5f*deltaTime);
	tlTurnEntityX(g_child, 36.0f*deltaTime);
	tlTurnEntityZ(g_plane, -50.0f*deltaTime);
	#endif

	while( tlEv_Pending() ) {
		tlEv_Next();

		if( cam_onEvent() ) {
			continue;
		}

		if( tlEscapeKey() ) {
			tlR_Fini();
			tlScr_Fini();
			return FALSE;
		}
	}

	speed = 2.0f;
	if( tlShiftKey() ) {
		speed *= 1.75f;
	}

	speed *= deltaTime;

	tlSetCameraEntity( tlR_DefaultCamera() );

#if 0
	if( tlUpKey() || tlKeyState( kTlKey_W ) ) {
		tlMoveCameraZ( speed );
	} else if( tlDownKey() || tlKeyState( kTlKey_S ) ) {
		tlMoveCameraZ( -speed );
	}

	if( tlLeftKey() ) {
		tlTurnCameraY( -speed*12.0f );
	} else if( tlRightKey() ) {
		tlTurnCameraY( speed*12.0f );
	}

	if( tlKeyState( kTlKey_A ) ) {
		tlMoveCameraX( -speed );
	} else if( tlKeyState( kTlKey_D ) ) {
		tlMoveCameraX( speed );
	}
#else
	cam_onFrame( deltaTime );
#endif

	return TRUE;
}

double getTime( void )
{
	return ( ( double )tlSys_Microtime() )/1000000.0;
}

int main() {
	double newTime, oldTime, deltaTime;

	/* initialization */
	tlInit();
	
	/* version control info */
#if BUILDGEN_GITINFO_AVAILABLE
	printf( "git-info:\n" );
	printf( "\tbranch: %s\n", BUILDGEN_GITINFO_BRANCH );
	printf( "\tcommit: %s\n", BUILDGEN_GITINFO_COMMIT );
	printf( "\ttstamp: %s\n", BUILDGEN_GITINFO_TSTAMP );
	printf( "\n" );
#endif
	
	tlSetCameraAutoAspect(1280.0/720.0, kTlAspect_Fit);

	g_shape = tlNewFigureEightTorus(tlFirstBrush(), 1.0f, 36);

	g_cube = tlNewCube(tlFirstBrush(), 1.0f);
	tlSetEntityPosition(g_cube, 0.5f,0.0f,5.0f);

	g_child = tlNewCube(tlFirstBrush(), 0.25f);
	tlSetEntityPosition(g_child, 0.7f,0.7f,0.7f);
	tlSetEntityParent(g_child, g_cube);

	g_plane = tlNewPlane(tlFirstBrush(), 10.0f, 10.0f);
	tlSetEntityPosition(g_plane, 0.0f,-3.5f,10.0f);
	tlTurnEntityX(g_plane, 90.0f);

	tlSetEntityParentGlobal(g_cube, g_plane);

	/* main loop */
	do {
		/* update */
		if( !update( tlGetDeltaTime() ) ) {
			break;
		}
	} while( tlLoop() );

	/* done */
	return EXIT_SUCCESS;
}

