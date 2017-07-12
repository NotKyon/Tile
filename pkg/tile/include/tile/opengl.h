#ifndef TILE_OPENGL_H
#define TILE_OPENGL_H

#include "const.h"

TILE_EXTRNC_ENTER

/* OpenGL */
TlBool tlGL_IsExtensionSupported(const char *extension);
void tlGL_RequireExtension(const char *extension);

TlFn_t tlGL_Proc(const char *proc);

void tlGL__CheckErrorImpl_(const char *file, int line);
#if defined(DEBUG)||defined(_DEBUG)||defined(__debug__)
# define tlGL_CheckError() tlGL__CheckErrorImpl_(__FILE__,__LINE__)
#else
# define tlGL_CheckError()
#endif

TILE_EXTRNC_LEAVE

#endif

