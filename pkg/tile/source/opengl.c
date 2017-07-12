#include <tile/opengl.h>

/*
 * ==========================================================================
 *
 *	OPENGL FUNCTIONS
 *
 * ==========================================================================
 */
TlBool tlGL_IsExtensionSupported(const char *extension) {
#if GLFW_ENABLED
	return glfwExtensionSupported(extension) ? TRUE : FALSE;
#else
	const char *glexts;
	const char *p;
	size_t len;

	glexts = ( const char * )glGetString( GL_EXTENSIONS );
	if( !glexts ) {
		return FALSE;
	}

	len = strlen( extension );
	p = strstr( glexts, extension );
	if( !p || p[ len ] > ' ' ) {
		return FALSE;
	}

	return TRUE;
#endif
}
void tlGL_RequireExtension(const char *extension) {
	if (!tlGL_IsExtensionSupported(extension))
		tlErrorExit("Need GL extension \'%s\'", extension);
}

TlFn_t tlGL_Proc(const char *proc) {
	TlFnPtr x;

#if GLFW_ENABLED
	x.fn = ( TlFn_t )glfwGetProcAddress( proc );
#elif defined( _WIN32 )
	x.fn = ( TlFn_t )wglGetProcAddress( proc );
#else
# error Not implemented
#endif
	if( !x.p ) {
		tlErrorExit("GL function not found \'%s\'", proc);
	}

	return x.fn;
}

void tlGL__CheckErrorImpl_(const char *file, int line) {
	const char *err;

	switch(glGetError()) {
	case GL_NO_ERROR: return;
	case GL_INVALID_ENUM: err="GL_INVALID_ENUM"; break;
	case GL_INVALID_VALUE: err="GL_INVALID_VALUE"; break;
	case GL_INVALID_OPERATION: err="GL_INVALID_OPERATION"; break;
	case GL_STACK_OVERFLOW: err="GL_STACK_OVERFLOW"; break;
	case GL_STACK_UNDERFLOW: err="GL_STACK_UNDERFLOW"; break;
	case GL_OUT_OF_MEMORY: err="GL_OUT_OF_MEMORY"; break;
	default: err="(unknown)"; break;
	}

	tlErrorFileExit(file, line, "glGetError() = %s\n", err);
}

