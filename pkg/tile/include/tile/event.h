#ifndef TILE_EVENT_H
#define TILE_EVENT_H

#include "const.h"

TILE_EXTRNC_ENTER


/* keyboard key scancode */
typedef enum
{
	kTlKey_None             = 0,

	kTlKey_Escape           = 1,    /* escape key (US KBD) */
	kTlKey_One              = 2,    /* '1'/'!' key (US KBD) */
	kTlKey_Two              = 3,    /* '2'/'@' key (US KBD) */
	kTlKey_Three            = 4,    /* '3'/'#' key (US KBD) */
	kTlKey_Four             = 5,    /* '4'/'$' key (US KBD) */
	kTlKey_Five             = 6,    /* '5'/'%' key (US KBD) */
	kTlKey_Six              = 7,    /* '6'/'^' key (US KBD) */
	kTlKey_Seven            = 8,    /* '7'/'&' key (US KBD) */
	kTlKey_Eight            = 9,    /* '8'/'*' key (US KBD) */
	kTlKey_Nine             = 10,   /* '9'/'(' key (US KBD) */
	kTlKey_Zero             = 11,   /* '0'/')' key (US KBD) */
	kTlKey_Minus            = 12,   /* '-'/'_' key (US KBD) */
	kTlKey_Equals           = 13,   /* '='/'+' key (US KBD) */
	kTlKey_Back             = 14,   /* backspace key (US KBD) */
	kTlKey_Tab              = 15,   /* tab key (US KBD) */
	kTlKey_Q                = 16,   /* 'q'/'Q' key (US KBD) */
	kTlKey_W                = 17,   /* 'w'/'W' key (US KBD) */
	kTlKey_E                = 18,   /* 'e'/'E' key (US KBD) */
	kTlKey_R                = 19,   /* 'r'/'R' key (US KBD) */
	kTlKey_T                = 20,   /* 't'/'T' key (US KBD) */
	kTlKey_Y                = 21,   /* 'y'/'Y' key (US KBD) */
	kTlKey_U                = 22,   /* 'u'/'U' key (US KBD) */
	kTlKey_I                = 23,   /* 'i'/'I' key (US KBD) */
	kTlKey_O                = 24,   /* 'o'/'O' key (US KBD) */
	kTlKey_P                = 25,   /* 'p'/'P' key (US KBD) */
	kTlKey_LBracket         = 26,   /* '['/'{' key (US KBD) */
	kTlKey_RBracket         = 27,   /* ']'/'}' key (US KBD) */
	kTlKey_Return           = 28,   /* enter/return key (US KBD) */
	kTlKey_LControl         = 29,   /* left control key (US KBD) */
	kTlKey_A                = 30,   /* 'a'/'A' key (US KBD) */
	kTlKey_S                = 31,   /* 's'/'S' key (US KBD) */
	kTlKey_D                = 32,   /* 'd'/'D' key (US KBD) */
	kTlKey_F                = 33,   /* 'f'/'F' key (US KBD) */
	kTlKey_G                = 34,   /* 'g'/'G' key (US KBD) */
	kTlKey_H                = 35,   /* 'h'/'H' key (US KBD) */
	kTlKey_J                = 36,   /* 'j'/'J' key (US KBD) */
	kTlKey_K                = 37,   /* 'k'/'K' key (US KBD) */
	kTlKey_L                = 38,   /* 'l'/'L' key (US KBD) */
	kTlKey_Semicolon        = 39,   /* ';'/':' key (US KBD) */
	kTlKey_Apostrophe       = 40,   /* '''/'"' key (US KBD) */
	kTlKey_Grave            = 41,   /* */
	kTlKey_LShift           = 42,   /* left shift key (US KBD) */
	kTlKey_Backslash        = 43,   /* '\'/'|' key (US KBD) */
	kTlKey_Z                = 44,   /* 'z'/'Z' key (US KBD) */
	kTlKey_X                = 45,   /* 'x'/'X' key (US KBD) */
	kTlKey_C                = 46,   /* 'c'/'C' key (US KBD) */
	kTlKey_V                = 47,   /* 'v'/'V' key (US KBD) */
	kTlKey_B                = 48,   /* 'b'/'B' key (US KBD) */
	kTlKey_N                = 49,   /* 'n'/'N' key (US KBD) */
	kTlKey_M                = 50,   /* 'm'/'M' key (US KBD) */
	kTlKey_Comma            = 51,   /* ','/'<' key (US KBD) */
	kTlKey_Period           = 52,   /* '.'/'>' key (US KBD) */
	kTlKey_Slash            = 53,   /* '/'/'?' key (US KBD) */
	kTlKey_RShift           = 54,   /* right shift key (US KBD) */
	kTlKey_Multiply         = 55,   /* '*' (numpad) key (US KBD) */
	kTlKey_LMenu            = 56,   /* left alt key (US KBD) */
	kTlKey_Space            = 57,   /* space bar key (US KBD) */
	kTlKey_Capital          = 58,   /* caps-lock key (US KBD) */
	kTlKey_F1               = 59,   /* F1 key (US KBD) */
	kTlKey_F2               = 60,   /* F2 key (US KBD) */
	kTlKey_F3               = 61,   /* F3 key (US KBD) */
	kTlKey_F4               = 62,   /* F4 key (US KBD) */
	kTlKey_F5               = 63,   /* F5 key (US KBD) */
	kTlKey_F6               = 64,   /* F6 key (US KBD) */
	kTlKey_F7               = 65,   /* F7 key (US KBD) */
	kTlKey_F8               = 66,   /* F8 key (US KBD) */
	kTlKey_F9               = 67,   /* F9 key (US KBD) */
	kTlKey_F10              = 68,   /* F10 key (US KBD) */
	kTlKey_NumLock          = 69,   /* num-lock key (US KBD) */
	kTlKey_Scroll           = 70,   /* scroll-lock key (US KBD) */
	kTlKey_NumPad7          = 71,   /* '7' (numpad) key (US KBD) */
	kTlKey_NumPad8          = 72,   /* '8' (numpad) key (US KBD) */
	kTlKey_NumPad9          = 73,   /* '9' (numpad) key (US KBD) */
	kTlKey_Subtract         = 74,   /* '-' (numpad) key (US KBD) */
	kTlKey_NumPad4          = 75,   /* '4' (numpad) key (US KBD) */
	kTlKey_NumPad5          = 76,   /* '5' (numpad) key (US KBD) */
	kTlKey_NumPad6          = 77,   /* '6' (numpad) key (US KBD) */
	kTlKey_Add              = 78,   /* '+' (numpad) key (US KBD) */
	kTlKey_NumPad1          = 79,   /* '1' (numpad) key (US KBD) */
	kTlKey_NumPad2          = 80,   /* '2' (numpad) key (US KBD) */
	kTlKey_NumPad3          = 81,   /* '3' (numpad) key (US KBD) */
	kTlKey_NumPad0          = 82,   /* '0' (numpad) key (US KBD) */
	kTlKey_Decimal          = 83,   /* '.' (numpad) key (US KBD) */
	kTlKey_Oem102           = 86,   /* */
	kTlKey_F11              = 87,   /* F11 key (US KBD) */
	kTlKey_F12              = 88,   /* F12 key (US KBD) */
	kTlKey_F13              = 100,  /* F13 key (US KBD) */
	kTlKey_F14              = 101,  /* F14 key (US KBD) */
	kTlKey_F15              = 102,  /* F15 key (US KBD) */
	kTlKey_Kana             = 112,  /* */
	kTlKey_AbntC1           = 115,  /* */
	kTlKey_Convert          = 121,  /* */
	kTlKey_NoConvert        = 123,  /* */
	kTlKey_Yen              = 125,  /* */
	kTlKey_AbntC2           = 126,  /* */
	kTlKey_NumPadEquals     = 141,  /* '=' (numpad) key (US KBD) */
	kTlKey_PrevTrack        = 144,  /* */
	kTlKey_At               = 145,  /* '@' (alternate) key (US KBD) */
	kTlKey_Colon            = 146,  /* ':' (alternate) key (US KBD) */
	kTlKey_Underline        = 147,  /* '_' (alternate) key (US KBD) */
	kTlKey_Kanji            = 148,  /* */
	kTlKey_Stop             = 149,  /* */
	kTlKey_Ax               = 150,  /* */
	kTlKey_Unlabeled        = 151,  /* */
	kTlKey_NextTrack        = 153,  /* */
	kTlKey_NumPadEnter      = 156,  /* enter (numpad) key (US KBD) */
	kTlKey_RControl         = 157,  /* right control key (US KBD) */
	kTlKey_Mute             = 160,  /* */
	kTlKey_Calculator       = 161,  /* */
	kTlKey_PlayPause        = 162,  /* */
	kTlKey_MediaStop        = 164,  /* */
	kTlKey_VolumeDown       = 174,  /* */
	kTlKey_VolumeUp         = 176,  /* */
	kTlKey_WebHome          = 178,  /* */
	kTlKey_NumPadComma      = 179,  /* ',' (numpad) key (US KBD) */
	kTlKey_Divide           = 181,  /* '/' (numpad) key (US KBD) */
	kTlKey_SysRq            = 183,  /* */
	kTlKey_RMenu            = 184,  /* right alt key (US KBD) */
	kTlKey_Pause            = 197,  /* pause key (US KBD) */
	kTlKey_Home             = 199,  /* home key (US KBD) */
	kTlKey_Up               = 200,  /* up arrow key (US KBD) */
	kTlKey_Prior            = 201,  /* page up key (US KBD) */
	kTlKey_Left             = 203,  /* left arrow key (US KBD) */
	kTlKey_Right            = 205,  /* right arrow key (US KBD) */
	kTlKey_End              = 207,  /* end key (US KBD) */
	kTlKey_Down             = 208,  /* down key (US KBD) */
	kTlKey_Next             = 209,  /* page down key (US KBD) */
	kTlKey_Insert           = 210,  /* insert key (US KBD) */
	kTlKey_Delete           = 211,  /* delete key (US KBD) */
	kTlKey_LWin             = 219,  /* left Windows key (US KBD) */
	kTlKey_RWin             = 220,  /* right Windows key (US KBD) */
	kTlKey_Apps             = 221,  /* */
	kTlKey_Power            = 222,  /* */
	kTlKey_Sleep            = 223,  /* */
	kTlKey_Wake             = 227,  /* */
	kTlKey_WebSearch        = 229,  /* */
	kTlKey_WebFavorites     = 230,  /* */
	kTlKey_WebRefresh       = 231,  /* */
	kTlKey_WebStop          = 232,  /* */
	kTlKey_WebForward       = 233,  /* */
	kTlKey_WebBack          = 234,  /* */
	kTlKey_MyComputer       = 235,  /* */
	kTlKey_Mail             = 236,  /* */
	kTlKey_MediaSelect      = 237,  /* */

	kTlKey_Esc              = kTlKey_Escape,
	kTlKey_Backspace        = kTlKey_Back,
	kTlKey_NumPadStar       = kTlKey_Multiply,
	kTlKey_LAlt             = kTlKey_LMenu,
	kTlKey_CapsLock         = kTlKey_Capital,
	kTlKey_NumPadMinus      = kTlKey_Subtract,
	kTlKey_NumPadPlus       = kTlKey_Add,
	kTlKey_NumPadPeriod     = kTlKey_Decimal,
	kTlKey_NumPadSlash      = kTlKey_Divide,
	kTlKey_RAlt             = kTlKey_RMenu,
	kTlKey_UpArrow          = kTlKey_Up,
	kTlKey_PgUp             = kTlKey_Prior,
	kTlKey_LeftArrow        = kTlKey_Left,
	kTlKey_RightArrow       = kTlKey_Right,
	kTlKey_DownArrow        = kTlKey_Down,
	kTlKey_PgDn             = kTlKey_Next,
	kTlKey_Circumflex       = kTlKey_PrevTrack,
	kTlKey_LCtrl            = kTlKey_LControl,
	kTlKey_RCtrl            = kTlKey_RControl,
	kTlKey_LSuper           = kTlKey_LWin,
	kTlKey_RSuper           = kTlKey_RWin,
	kTlKey_LCommand         = kTlKey_LWin,
	kTlKey_RCommand         = kTlKey_RWin,
	kTlKey_LCmd             = kTlKey_LWin,
	kTlKey_RCmd             = kTlKey_RWin,
	kTlKey_LApple           = kTlKey_LWin,
	kTlKey_RApple           = kTlKey_RWin,
	kTlKey_Enter            = kTlKey_Return
} TlKey_t;
typedef enum
{
	kTlMouse_None,

	kTlMouse1,
	kTlMouse2,
	kTlMouse3,
	kTlMouse4,
	kTlMouse5
} TlMouse_t;

typedef enum
{
	kTlMF_LShift   = 1<<0,
	kTlMF_RShift   = 1<<1,
	kTlMF_LAlt     = 1<<2,
	kTlMF_RAlt     = 1<<3,
	kTlMF_LControl = 1<<4,
	kTlMF_RControl = 1<<5,
	kTlMF_Mouse1   = 1<<6,
	kTlMF_Mouse2   = 1<<7,
	kTlMF_Mouse3   = 1<<8,
	kTlMF_Mouse4   = 1<<9,
	kTlMF_Mouse5   = 1<<10
} TlModflag_t;

typedef enum
{
	kTlEv_None,

	/* key was pressed ;; TlEvent{key,mods,isRepeat} */
	kTlEv_KeyPress,
	/* key was released ;; TlEvent{key,mods} */
	kTlEv_KeyRelease,
	/* key was pressed, generating a text character ;; TlEvent{utf32Char} */
	kTlEv_KeyChar,

	/* mouse button was pressed ;; TlEvent{mouse,mods,x,y} */
	kTlEv_MousePress,
	/* mouse button was released ;; TlEvent{mouse,mods,x,y} */
	kTlEv_MouseRelease,
	/* mouse wheel was scrolled ;; TlEvent{mods,x,y,delta} */
	kTlEv_MouseWheel,
	/* mouse was moved ;; TlEvent{mods,x,y} ;; (x,y) is the new position */
	kTlEv_MouseMove,
	/* mouse left window ;; TlEvent{mods} */
	kTlEv_MouseExit
} TlEventType_t;

typedef struct TlEvent_s
{
	TlEventType_t type;

	TlKey_t       key;
	TlMouse_t     mouse;

	TlU32         mods;
	TlBool        isRepeat;

	TlU32         utf32Char;

	TlS32         x;
	TlS32         y;
	float         delta;
} TlEvent;

void tlEv_Init( void );
void tlEv_Fini( void );

void tlEv_EmitEvent( const TlEvent *pInEvent );
void tlEv_EmitKeyPress( TlKey_t key, TlU32 mods, TlBool isRepeat );
void tlEv_EmitKeyRelease( TlKey_t key, TlU32 mods );
void tlEv_EmitKeyChar( TlU32 utf32Char );
void tlEv_EmitMousePress( TlMouse_t mouse, TlU32 mods, TlS32 x, TlS32 y );
void tlEv_EmitMouseRelease( TlMouse_t mouse, TlU32 mods, TlS32 x, TlS32 y );
void tlEv_EmitMouseWheel( TlU32 mods, TlS32 x, TlS32 y, float delta );
void tlEv_EmitMouseMove( TlU32 mods, TlS32 newX, TlS32 newY );
void tlEv_EmitMouseExit( TlU32 mods );

TlBool tlEv_Pending( void );
const TlEvent *tlEv_Peek( void );
const TlEvent *tlEv_Next( void );
const TlEvent *tlEv_Current( void );

void tlSetKeyState( TlKey_t key, TlBool state );
void tlSetMouseState( TlMouse_t mouse, TlBool state );

TlKey_t tlScancode( void );
TlBool tlKeyState( TlKey_t key );
TlU32 tlKeyPressed( TlKey_t key );
TlU32 tlKeyReleased( TlKey_t key );

TlMouse_t tlMouseClick( void );
TlBool tlMouseState( TlMouse_t mouse );
TlU32 tlMousePressed( TlMouse_t mouse );
TlU32 tlMouseReleased( TlMouse_t mouse );
TlS32 tlMouseX( void );
TlS32 tlMouseY( void );
TlS32 tlMouseMoveX( void );
TlS32 tlMouseMoveY( void );
void tlClearMouseMove( void );
float tlMouseWheel( void );
void tlClearMouseWheel( void );

TlBool tlShiftKey( void );
TlBool tlControlKey( void );
TlBool tlAltKey( void );

TlBool tlEscapeKey( void );

TlBool tlLeftKey( void );
TlBool tlUpKey( void );
TlBool tlRightKey( void );
TlBool tlDownKey( void );

TILE_EXTRNC_LEAVE

#endif
