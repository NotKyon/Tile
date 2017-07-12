#ifndef TILE_CONSOLE_H
#define TILE_CONSOLE_H

#include "const.h"
#include "math.h"

TILE_EXTRNC_ENTER

#define TL_DEVCON_KEY '`' /*96*/

enum {
	kTlConColor_Background,			/*background color of the console*/
	kTlConColor_Foreground,			/*normal foreground text*/
	kTlConColor_Error,				/*error color*/
	kTlConColor_Warning,			/*warning color*/
	kTlConColor_Prompt,				/*color of the prompt in the edit field*/
	kTlConColor_EditField,			/*what you type into...*/
	kTlConColor_Autocomplete,		/*within edit-field*/
	kTlConColor_Cursor,

	kTlNumConColors
};

#define TL_DEVCON_COLUMNS				80
#define TL_DEVCON_ROWS					0x4000
#define TL_DEVCON_TEXT					(TL_DEVCON_COLUMNS*TL_DEVCON_ROWS)
#define TL_DEVCON_COMMAND				0x100
#define TL_DEVCON_PROMPT				0x20

typedef struct TlDevcon_s {
	TlColor palette[kTlNumConColors];
	int curPalette;					/*current palette color index*/

	int visible;					/*is the console visible at all?*/
	int height;						/*height of the console in pixels*/

	int curRow;						/*which row are we currently viewing? (bottom)*/
	int visRows;					/*how many rows are visible?*/
	int numRows;					/*how many rows have been written?*/

	int curText;					/*index into the text buffer*/

	unsigned short text[TL_DEVCON_TEXT]; /*text buffer*/

	char prompt[TL_DEVCON_PROMPT];		/*prompt*/
	char command[TL_DEVCON_COMMAND];	/*command buffer*/
	char autocomplete[TL_DEVCON_COMMAND]; /*autocomplete*/

	int curCommand;					/*current index in the command buffer*/

	GLuint fontImage;				/*built-in font*/
} TlDevcon;

extern int tl__g_devconAlive;
extern TlDevcon tl__g_devcon;

extern unsigned char tl__g_devconFont[128*128]; /*GL_LUMINANCE*/

/*
 * Initialize the developer console system. Call this BEFORE any other developer
 * console calls!
 * NOTE: Initializing the developer console does not automatically show it. The
 *       developer console is hidden by default.
 */
void tlCon_Init();

/*
 * Finish using the developer console system.
 */
void tlCon_Fini();

/*
 * Determine whether the developer console has been initialized.
 *
 * return: 1 if the developer console has been initialized; 0 otherwise.
 */
int tlCon_IsAlive();

/*
 * Show the developer console.
 */
void tlCon_Show();

/*
 * Hide the developer console.
 */
void tlCon_Hide();

/*
 * Determine whether the developer console is visible.
 *
 * return: 1 if the developer console is visible; 0 otherwise.
 */
TlBool tlCon_IsVisible();

/*
 * Render the developer console using OpenGL.
 */
void tlCon_Render();

/*
 * Print to the developer console using printf style formatting. Special
 * character codes exist in the string to indicate which color to use for the
 * text. (If no character code is specified, the default foreground color is
 * assumed.)
 *
 * format: Formatting string. This has the same attributes as printf.
 * args: Argument list for the format.
 *
 * Color Codes
 * -----------
 * "^0" = Foreground
 * "^1" = tlError
 * "^2" = Warning
 * "^3" = Prompt
 * "^4" = Edit Field
 * "^5" = Autocomplete
 * "^6" = Cursor
 */
void tlCon_PrintV(const char *format, va_list args);

/*
 * Print to the developer console using con_PrintV() without an explicit args
 * parameter.
 */
void tlCon_Print(const char *format, ...);

/*
 * Key handling
 *
 * Uses the current event. (see event.h)
 */
void tlCon_KeyEvent();

/*
 * Mouse event handling
 *
 * Uses the current event. (see event.h)
 */
void tlCon_MouseEvent();

TILE_EXTRNC_LEAVE

#endif
