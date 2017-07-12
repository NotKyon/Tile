#include <tile/event.h>

typedef struct evlink_s
{
	TlEvent				event;
	struct evlink_s *	prev;
	struct evlink_s *	next;
} evlink_t;

static evlink_t *g_ev_head = ( evlink_t * )0;
static evlink_t *g_ev_tail = ( evlink_t * )0;
static TlEvent g_ev_curr;

static TlU32 g_ev_keyPresses[ 256 ];
static TlU32 g_ev_keyReleases[ 256 ];
static TlBool g_ev_keyState[ 256 ];
static TlKey_t g_ev_lastKey;

static TlU32 g_ev_mousePresses[ 5 ];
static TlU32 g_ev_mouseReleases[ 5 ];
static TlBool g_ev_mouseState[ 5 ];
static TlS32 g_ev_mousePos[ 2 ], g_ev_mouseRefPos[ 2 ];
static float g_ev_mouseScroll;
static TlMouse_t g_ev_lastMouse;

static TlBool g_ev_needPeek;
static TlU32 g_ev_mods;

static TlU8 g_ev_memory[ 1024*sizeof( evlink_t ) ];
static size_t g_ev_memoryPos = 0;

evlink_t *tlNewEvlink( const TlEvent *pInEvent )
{
	evlink_t *p;

	if( g_ev_memoryPos + sizeof( evlink_t ) > sizeof( g_ev_memory ) ) {
		return ( evlink_t * )0;
	}

	p = ( evlink_t * )&g_ev_memory[ g_ev_memoryPos ];
	g_ev_memoryPos += sizeof( evlink_t );

	if( pInEvent != ( TlEvent * )0 ) {
		p->event = *pInEvent;
	} else {
		memset( &p->event, 0, sizeof( TlEvent ) );
	}

	p->prev = g_ev_tail;
	p->next = ( evlink_t * )0;
	if( g_ev_tail != ( evlink_t * )0 ) {
		g_ev_tail->next = p;
	} else {
		g_ev_head = p;
	}
	g_ev_tail = p;

	return p;
}
evlink_t *tlDeleteEvlink( evlink_t *p )
{
	if( !p ) {
		return ( evlink_t * )0;
	}

	if( p->prev != ( evlink_t * )0 ) {
		p->prev->next = p->next;
	} else {
		g_ev_head = p->next;
	}
	if( p->next != ( evlink_t * )0 ) {
		p->next->prev = p->prev;
	} else {
		g_ev_tail = p->prev;
	}

	if( !g_ev_head ) {
		g_ev_memoryPos = 0;
	}

	return ( evlink_t * )0;
}

void tlEv_Init( void )
{
	memset( &g_ev_curr, 0, sizeof( g_ev_curr ) );

	memset( &g_ev_keyPresses[ 0 ], 0, sizeof( g_ev_keyPresses ) );
	memset( &g_ev_keyReleases[ 0 ], 0, sizeof( g_ev_keyReleases ) );
	memset( &g_ev_keyState[ 0 ], 0, sizeof( g_ev_keyState ) );
	g_ev_lastKey = kTlKey_None;

	memset( &g_ev_mousePresses[ 0 ], 0, sizeof( g_ev_mousePresses ) );
	memset( &g_ev_mouseReleases[ 0 ], 0, sizeof( g_ev_mouseReleases ) );
	memset( &g_ev_mouseState[ 0 ], 0, sizeof( g_ev_mouseState ) );
	memset( &g_ev_mousePos[ 0 ], 0, sizeof( g_ev_mousePos ) );
	memset( &g_ev_mouseRefPos[ 0 ], 0, sizeof( g_ev_mouseRefPos ) );
	g_ev_lastMouse = kTlMouse_None;
	g_ev_mouseScroll = 0.0f;

	g_ev_needPeek = TRUE;
	g_ev_mods = 0;
}
void tlEv_Fini( void )
{
	while( g_ev_head != ( evlink_t * )0 ) {
		tlDeleteEvlink( g_ev_head );
	}
}

void tlEv_EmitEvent( const TlEvent *pInEvent )
{
	( void )tlNewEvlink( pInEvent );
}

void tlEv_EmitKeyPress( TlKey_t key, TlU32 mods, TlBool isRepeat )
{
	evlink_t *p;

	p = tlNewEvlink( ( TlEvent * )0 );
	if( !p ) {
		return;
	}

	p->event.type = kTlEv_KeyPress;
	p->event.key = key;
	p->event.mods = mods;
	p->event.isRepeat = FALSE & isRepeat;
}
void tlEv_EmitKeyRelease( TlKey_t key, TlU32 mods )
{
	evlink_t *p;

	p = tlNewEvlink( ( TlEvent * )0 );
	if( !p ) {
		return;
	}

	p->event.type = kTlEv_KeyRelease;
	p->event.key = key;
	p->event.mods = mods;
}
void tlEv_EmitKeyChar( TlU32 utf32Char )
{
	evlink_t *p;

	p = tlNewEvlink( ( TlEvent * )0 );
	if( !p ) {
		return;
	}

	p->event.type = kTlEv_KeyChar;
	p->event.utf32Char = utf32Char;
}
void tlEv_EmitMousePress( TlMouse_t mouse, TlU32 mods, TlS32 x, TlS32 y )
{
	evlink_t *p;

	p = tlNewEvlink( ( TlEvent * )0 );
	if( !p ) {
		return;
	}

	p->event.type = kTlEv_MousePress;
	p->event.mouse = mouse;
	p->event.mods = mods;
	p->event.x = x;
	p->event.y = y;
}
void tlEv_EmitMouseRelease( TlMouse_t mouse, TlU32 mods, TlS32 x, TlS32 y )
{
	evlink_t *p;

	p = tlNewEvlink( ( TlEvent * )0 );
	if( !p ) {
		return;
	}

	p->event.type = kTlEv_MouseRelease;
	p->event.mouse = mouse;
	p->event.mods = mods;
	p->event.x = x;
	p->event.y = y;
}
void tlEv_EmitMouseWheel( TlU32 mods, TlS32 x, TlS32 y, float delta )
{
	evlink_t *p;

	p = tlNewEvlink( ( TlEvent * )0 );
	if( !p ) {
		return;
	}

	p->event.type = kTlEv_MouseWheel;
	p->event.mods = mods;
	p->event.x = x;
	p->event.y = y;
	p->event.delta = delta;
}
void tlEv_EmitMouseMove( TlU32 mods, TlS32 newX, TlS32 newY )
{
	evlink_t *p;

	p = tlNewEvlink( ( TlEvent * )0 );
	if( !p ) {
		return;
	}

	p->event.type = kTlEv_MouseMove;
	p->event.mods = mods;
	p->event.x = newX;
	p->event.y = newY;
}
void tlEv_EmitMouseExit( TlU32 mods )
{
	evlink_t *p;

	p = tlNewEvlink( ( TlEvent * )0 );
	if( !p ) {
		return;
	}

	p->event.type = kTlEv_MouseExit;
	p->event.mods = mods;
}

TlBool tlEv_Pending( void )
{
	return g_ev_head != ( evlink_t * )0 ? TRUE : FALSE;
}

static void tlApplyMouse( TlS32 x, TlS32 y )
{
	g_ev_mousePos[ 0 ] = x;
	g_ev_mousePos[ 1 ] = y;
}
static void tlProcessEvent( const TlEvent *pNew )
{
	if( pNew->type == kTlEv_KeyPress && !pNew->isRepeat ) {
		g_ev_lastKey = pNew->key;
		++g_ev_keyPresses[ ( TlU8 )pNew->key ];
		g_ev_keyState[ ( TlU8 )pNew->key ] = TRUE;
		g_ev_mods = pNew->mods;
		return;
	}

	if( pNew->type == kTlEv_KeyRelease ) {
		g_ev_lastKey = kTlKey_None;
		--g_ev_keyReleases[ ( TlU8 )pNew->key ];
		g_ev_keyState[ ( TlU8 )pNew->key ] = FALSE;
		g_ev_mods = pNew->mods;
		return;
	}

	if( pNew->type == kTlEv_MousePress ) {
		g_ev_lastMouse = pNew->mouse;
		++g_ev_mousePresses[ ( ( TlU8 )pNew->mouse - 1 )%5 ];
		g_ev_mouseState[ ( ( TlU8 )pNew->mouse - 1 )%5 ] = TRUE;
		tlApplyMouse( pNew->x, pNew->y );
		g_ev_mods = pNew->mods;
		return;
	}
	if( pNew->type == kTlEv_MouseRelease ) {
		g_ev_lastMouse = kTlMouse_None;
		++g_ev_mouseReleases[ ( ( TlU8 )pNew->mouse - 1 )%5 ];
		g_ev_mouseState[ ( ( TlU8 )pNew->mouse - 1 )%5 ] = FALSE;
		tlApplyMouse( pNew->x, pNew->y );
		g_ev_mods = pNew->mods;
		return;
	}
	if( pNew->type == kTlEv_MouseMove ) {
		tlApplyMouse( pNew->x, pNew->y );
		g_ev_mods = pNew->mods;
		return;
	}

	if( pNew->type == kTlEv_MouseWheel ) {
		tlApplyMouse( pNew->x, pNew->y );
		g_ev_mouseScroll = pNew->delta;
		g_ev_mods = pNew->mods;
		return;
	}
}

const TlEvent *tlEv_Peek( void )
{
	if( g_ev_head != ( evlink_t * )0 ) {
		tlProcessEvent( &g_ev_head->event );
		g_ev_curr = g_ev_head->event;
		g_ev_needPeek = FALSE;
		return &g_ev_curr;
	}

	return ( TlEvent * )0;
}
const TlEvent *tlEv_Next( void )
{
	const TlEvent *p;

	if( g_ev_needPeek ) {
		p = tlEv_Peek();
	} else if( g_ev_curr.type != kTlEv_None ) {
		p = &g_ev_curr;
	} else {
		p = ( TlEvent * )0;
	}

	g_ev_needPeek = TRUE;
	tlDeleteEvlink( g_ev_head );
	if( !p ) {
		return ( TlEvent * )0;
	}

	return p;
}
const TlEvent *tlEv_Current( void )
{
	return &g_ev_curr;
}

void tlSetKeyState( TlKey_t key, TlBool state )
{
	g_ev_keyState[ ( TlU8 )key ] = state;
}
void tlSetMouseState( TlMouse_t mouse, TlBool state )
{
	g_ev_mouseState[ ( ( ( TlU8 )mouse ) - 1 )%5 ] = state;
}

TlKey_t tlScancode( void )
{
	return g_ev_lastKey;
}
TlBool tlKeyState( TlKey_t key )
{
	return g_ev_keyState[ ( TlU8 )key ];
}
TlU32 tlKeyPressed( TlKey_t key )
{
	TlU32 r;

	r = g_ev_keyPresses[ ( TlU8 )key ];
	g_ev_keyPresses[ ( TlU8 )key ] = 0;

	return r;
}
TlU32 tlKeyReleased( TlKey_t key )
{
	TlU32 r;

	r = g_ev_keyReleases[ ( TlU8 )key ];
	g_ev_keyReleases[ ( TlU8 )key ] = 0;

	return r;
}

TlMouse_t tlMouseClick( void )
{
	return g_ev_lastMouse;
}
TlBool tlMouseState( TlMouse_t mouse )
{
	return g_ev_mouseState[ ( ( TlU8 )mouse - 1 )%5 ];
}
TlU32 tlMousePressed( TlMouse_t mouse )
{
	TlU32 r;

	r = g_ev_mousePresses[ ( ( TlU8 )mouse - 1 )%5 ];
	g_ev_mousePresses[ ( ( TlU8 )mouse - 1 )%5 ] = 0;

	return r;
}
TlU32 tlMouseReleased( TlMouse_t mouse )
{
	TlU32 r;

	r = g_ev_mouseReleases[ ( ( TlU8 )mouse - 1 )%5 ];
	g_ev_mouseReleases[ ( ( TlU8 )mouse - 1 )%5 ] = 0;

	return r;
}
TlS32 tlMouseX( void )
{
	return g_ev_mousePos[ 0 ];
}
TlS32 tlMouseY( void )
{
	return g_ev_mousePos[ 1 ];
}
TlS32 tlMouseMoveX( void )
{
	return g_ev_mousePos[ 0 ] - g_ev_mouseRefPos[ 0 ];
}
TlS32 tlMouseMoveY( void )
{
	return g_ev_mousePos[ 1 ] - g_ev_mouseRefPos[ 1 ];
}
void tlClearMouseMove( void )
{
	g_ev_mouseRefPos[ 0 ] = g_ev_mousePos[ 0 ];
	g_ev_mouseRefPos[ 1 ] = g_ev_mousePos[ 1 ];
}
float tlMouseWheel( void )
{
	return g_ev_mouseScroll;
}
void tlClearMouseWheel( void )
{
	g_ev_mouseScroll = 0.0f;
}

TlBool tlShiftKey( void )
{
	return ( g_ev_mods & ( kTlMF_LShift | kTlMF_RShift ) ) != 0 ? TRUE : FALSE;
}
TlBool tlControlKey( void )
{
	return ( g_ev_mods & ( kTlMF_LControl | kTlMF_RControl ) ) != 0 ? TRUE : FALSE;
}
TlBool tlAltKey( void )
{
	return ( g_ev_mods & ( kTlMF_LAlt | kTlMF_RAlt ) ) != 0 ? TRUE : FALSE;
}

TlBool tlEscapeKey( void )
{
	return tlKeyReleased( kTlKey_Escape );
}

TlBool tlLeftKey( void )
{
	return tlKeyState( kTlKey_Left );
}
TlBool tlUpKey( void )
{
	return tlKeyState( kTlKey_Up );
}
TlBool tlRightKey( void )
{
	return tlKeyState( kTlKey_Right );
}
TlBool tlDownKey( void )
{
	return tlKeyState( kTlKey_Down );
}
