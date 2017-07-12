#ifndef TILE_RENDERER_H
#define TILE_RENDERER_H

#include "const.h"

TILE_EXTRNC_ENTER

/*
 * ---------------
 * Renderer System
 * ---------------
 */
struct TlView_s;
struct TlEntity_s;

typedef struct TL_CACHELINE_ALIGNED TlRenderer_s {
	/*
	 * FUNCTIONS
	 */

	/* general */
	void(APIENTRY *ClearColor)(float r, float g, float b, float a);
	void(APIENTRY *ClearDepth)(float z);
	void(APIENTRY *ClearStencil)(unsigned int mask);
	void(APIENTRY *Clear)(GLbitfield buffers);

	/* queries (1.5) */
	void(APIENTRY *GenQueries)(GLsizei n, GLuint *ids);
	void(APIENTRY *DeleteQueries)(GLsizei n, const GLuint *ids);
	GLboolean(APIENTRY *IsQuery)(GLuint id);
	void(APIENTRY *BeginQuery)(GLenum target, GLuint id);
	void(APIENTRY *EndQuery)(GLenum target);
	void(APIENTRY *GetQueryiv)(GLenum target, GLenum pname, GLint *params);
	void(APIENTRY *GetQueryObjectiv)(GLuint id, GLenum pname, GLint *params);
	void(APIENTRY *GetQueryObjectuiv)(GLuint id, GLenum pname, GLuint *params);

	/* buffers (1.5) */
	void(APIENTRY *BindBuffer)(GLenum target, GLuint buffer);
	void(APIENTRY *DeleteBuffers)(GLsizei n, const GLuint *buffers);
	void(APIENTRY *GenBuffers)(GLsizei n, GLuint *buffers);
	GLboolean(APIENTRY *IsBuffer)(GLuint buffer);
	void(APIENTRY *BufferData)(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
	void(APIENTRY *BufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
	void(APIENTRY *GetBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data);
	GLvoid*(APIENTRY *MapBuffer)(GLenum target, GLenum access);
	GLboolean(APIENTRY *UnmapBuffer)(GLenum target);
	void(APIENTRY *GetBufferParameteriv)(GLenum target, GLenum pname, GLint *params);
	void(APIENTRY *GetBufferPointerv)(GLenum target, GLenum pname, GLvoid* *params);

	/* shaders (2.0) */
	void(APIENTRY *AttachShader)(GLuint program, GLuint shader);
	void(APIENTRY *BindAttribLocation)(GLuint program, GLuint index, const GLchar *name);
	void(APIENTRY *CompileShader)(GLuint shader);
	GLuint(APIENTRY *CreateProgram)(void);
	GLuint(APIENTRY *CreateShader)(GLenum type);
	void(APIENTRY *DeleteProgram)(GLuint program);
	void(APIENTRY *DeleteShader)(GLuint shader);
	void(APIENTRY *DetachShader)(GLuint program, GLuint shader);
	void(APIENTRY *DisableVertexAttribArray)(GLuint index);
	void(APIENTRY *EnableVertexAttribArray)(GLuint index);
	void(APIENTRY *GetActiveAttrib)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
	void(APIENTRY *GetActiveUniform)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
	void(APIENTRY *GetAttachedShaders)(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *obj);
	GLint(APIENTRY *GetAttribLocation)(GLuint program, const GLchar *name);
	void(APIENTRY *GetProgramiv)(GLuint program, GLenum pname, GLint *params);
	void(APIENTRY *GetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
	void(APIENTRY *GetShaderiv)(GLuint shader, GLenum pname, GLint *params);
	void(APIENTRY *GetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
	void(APIENTRY *GetShaderSource)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
	GLint(APIENTRY *GetUniformLocation)(GLuint program, const GLchar *name);
	void(APIENTRY *GetUniformfv)(GLuint program, GLint location, GLfloat *params);
	void(APIENTRY *GetUniformiv)(GLuint program, GLint location, GLint *params);
	void(APIENTRY *GetVertexAttribdv)(GLuint index, GLenum pname, GLdouble *params);
	void(APIENTRY *GetVertexAttribfv)(GLuint index, GLenum pname, GLfloat *params);
	void(APIENTRY *GetVertexAttribiv)(GLuint index, GLenum pname, GLint *params);
	void(APIENTRY *GetVertexAttribPointerv)(GLuint index, GLenum pname, GLvoid* *pointer);
	GLboolean(APIENTRY *IsProgram)(GLuint program);
	GLboolean(APIENTRY *IsShader)(GLuint shader);
	void(APIENTRY *LinkProgram)(GLuint program);
	void(APIENTRY *ShaderSource)(GLuint shader, GLsizei count, const GLchar* const *string, const GLint *length);
	void(APIENTRY *UseProgram)(GLuint program);
	void(APIENTRY *Uniform1f)(GLint location, GLfloat v0);
	void(APIENTRY *Uniform2f)(GLint location, GLfloat v0, GLfloat v1);
	void(APIENTRY *Uniform3f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
	void(APIENTRY *Uniform4f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
	void(APIENTRY *Uniform1i)(GLint location, GLint v0);
	void(APIENTRY *Uniform2i)(GLint location, GLint v0, GLint v1);
	void(APIENTRY *Uniform3i)(GLint location, GLint v0, GLint v1, GLint v2);
	void(APIENTRY *Uniform4i)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
	void(APIENTRY *Uniform1fv)(GLint location, GLsizei count, const GLfloat *value);
	void(APIENTRY *Uniform2fv)(GLint location, GLsizei count, const GLfloat *value);
	void(APIENTRY *Uniform3fv)(GLint location, GLsizei count, const GLfloat *value);
	void(APIENTRY *Uniform4fv)(GLint location, GLsizei count, const GLfloat *value);
	void(APIENTRY *Uniform1iv)(GLint location, GLsizei count, const GLint *value);
	void(APIENTRY *Uniform2iv)(GLint location, GLsizei count, const GLint *value);
	void(APIENTRY *Uniform3iv)(GLint location, GLsizei count, const GLint *value);
	void(APIENTRY *Uniform4iv)(GLint location, GLsizei count, const GLint *value);
	void(APIENTRY *UniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	void(APIENTRY *UniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	void(APIENTRY *UniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	void(APIENTRY *ValidateProgram)(GLuint program);
	void(APIENTRY *VertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);

	/*
	 * VARIABLES
	 */

	/* console font */
	GLuint conFontImage;
	unsigned int conFontCellResX;
	unsigned int conFontCellResY;
	unsigned int conFontResX;
	unsigned int conFontResY;

	/* shaders */
	GLuint tileMap_vert;
	GLuint tileMap_frag;
	GLuint tileMap_prog;
} TlRenderer;

/* Renderer */
void tlR_Init( void );
void tlR_Fini( void );
struct TlEntity_s *tlR_DefaultCamera( void );

void tlR_DrawView(struct TlView_s *view);
void tlR_Frame(double time);

void tlR_DrawText(const char *asciitext, TlS32 x, TlS32 y, TlU32 w, TlU32 h);

GLuint tlR_LoadGLSL( GLuint shaderType, const char *pszSourceCode );
GLuint tlR_LinkGLSL( GLuint vertShader, GLuint fragShader );

GLuint tlR_CreateShader(GLenum shaderType);
void tlR_ShaderSource(GLuint shader, int numStrings, const char **strings,
	int *stringLengths);
void tlR_CompileShader(GLuint shader);
GLuint tlR_CreateProgram();
void tlR_AttachObject(GLuint program, GLuint shader);
void tlR_LinkProgram(GLuint program);
void tlR_UseProgram(GLuint program);

void tlR_GetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length,
	char *infoLog);
void tlR_GetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length,
	char *infoLog);

void tlR_DeleteShader(GLuint shader);
void tlR_DeleteProgram(GLuint program);

TILE_EXTRNC_LEAVE

#endif

