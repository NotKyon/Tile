#include <tile/renderer.h>
#include <tile/math.h>
#include <tile/render_queue.h>
#include <tile/opengl.h>
#include <tile/entity.h>
#include <tile/camera.h>
#include <tile/brush.h>
#include <tile/view.h>
#include <tile/window.h>
#include <tile/event.h>

#if GLFW_ENABLED
extern GLFWwindow *tl__g_window;
#endif

/*
 * ==========================================================================
 *
 *	RENDERER
 *
 * ==========================================================================
 */
static TlRenderer R;
static TlBool g_didInit = FALSE;
static TlEntity *g_defcam = ( TlEntity * )0;
extern unsigned char tl__g_devconFont[];

#if SHADERS_ENABLED
static const char *g_tileMap_vertSrc =
	"#version 120\n"
	"\n"
	"attribute vec3 vPosition;\n"
	"attribute vec2 vTexCoord;\n"
	"\n"
	"varying vec2 fTexCoord;\n"
	"\n"
	"uniform vec2 gTexOffset;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_Position = vec4( vPosition, 1.0 );\n"
	"	fTexCoord = vec2( vTexCoord.x + gTexOffset.x, 1.0 - ( vTexCoord.y + gTexOffset.y ) );\n"
	"}\n";
static const char *g_tileMap_fragSrc =
	"#version 120\n"
	"\n"
	"//precision highp float;\n"
	"\n"
	"varying vec2 fTexCoord;\n"
	"\n"
	"uniform sampler2D gTilesetTexture;\n"
	"uniform sampler2D gTilemapTexture;\n"
	"\n"
	"uniform vec2 gSetRes; //resolution of gTilesetTexture in tiles (1 set tile = n pixels)\n"
	"uniform vec2 gMapRes; //resolution of gTilemapTexture in tiles (1 map tile = 1 pixel)\n"
	"\n"
	"vec4 SampleTile( in vec2 st )\n"
	"{\n"
	"	vec2 index = texture2D( gTilemapTexture, st ).rg*255.0;\n"
	"	vec2 tcOffset = fract( st*gMapRes )/gSetRes;\n"
	"	vec2 tcBase = mod( index, gSetRes )/gSetRes;\n"
	"\n"
	"	return texture2D( gTilesetTexture, tcBase + tcOffset );\n"
	"}\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = SampleTile( fTexCoord );\n"
	"}\n";
#endif

void tlR_Init( void )
{
	TlBrush *brush;

	if (g_didInit)
		return;

	g_didInit = TRUE;
	atexit(&tlR_Fini);

	glFrontFace(GL_CW);

#if 0
	*(TlFn_t *)&R->ClearColor = (TlFn_t)glClearColor;
	*(TlFn_t *)&R->ClearDepth = (TlFn_t)glClearDepth;
	*(TlFn_t *)&R->ClearStencil = (TlFn_t)glClearStencil;
	*(TlFn_t *)&R->Clear = (TlFn_t)glClear;
#endif

#define P(x_) *(TlFn_t *)&R.x_ = tlGL_Proc("gl" #x_)
	P(GenQueries);
	P(DeleteQueries);
	P(IsQuery);
	P(BeginQuery);
	P(EndQuery);
	P(GetQueryiv);
	P(GetQueryObjectiv);
	P(GetQueryObjectuiv);

	P(BindBuffer);
	P(DeleteBuffers);
	P(GenBuffers);
	P(IsBuffer);
	P(BufferData);
	P(BufferSubData);
	P(GetBufferSubData);
	P(MapBuffer);
	P(UnmapBuffer);
	P(GetBufferParameteriv);
	P(GetBufferPointerv);

	P(AttachShader);
	P(BindAttribLocation);
	P(CompileShader);
	P(CreateProgram);
	P(CreateShader);
	P(DeleteProgram);
	P(DeleteShader);
	P(DetachShader);
	P(DisableVertexAttribArray);
	P(EnableVertexAttribArray);
	P(GetActiveAttrib);
	P(GetActiveUniform);
	P(GetAttachedShaders);
	P(GetAttribLocation);
	P(GetProgramiv);
	P(GetProgramInfoLog);
	P(GetShaderiv);
	P(GetShaderInfoLog);
	P(GetShaderSource);
	P(GetUniformLocation);
	P(GetUniformfv);
	P(GetUniformiv);
	P(GetVertexAttribdv);
	P(GetVertexAttribfv);
	P(GetVertexAttribiv);
	P(GetVertexAttribPointerv);
	P(IsProgram);
	P(IsShader);
	P(LinkProgram);
	P(ShaderSource);
	P(UseProgram);
	P(Uniform1f);
	P(Uniform2f);
	P(Uniform3f);
	P(Uniform4f);
	P(Uniform1i);
	P(Uniform2i);
	P(Uniform3i);
	P(Uniform4i);
	P(Uniform1fv);
	P(Uniform2fv);
	P(Uniform3fv);
	P(Uniform4fv);
	P(Uniform1iv);
	P(Uniform2iv);
	P(Uniform3iv);
	P(Uniform4iv);
	P(UniformMatrix2fv);
	P(UniformMatrix3fv);
	P(UniformMatrix4fv);
	P(ValidateProgram);
	P(VertexAttribPointer);
#undef P

	R.conFontResX = 128;
	R.conFontResY = 128;
	R.conFontCellResX = 8;
	R.conFontCellResY = 16;
	glGenTextures(1,  &R.conFontImage);

	glBindTexture(GL_TEXTURE_2D, R.conFontImage);
	{
		TlU32 x, y;
		TlU8 *temp;
		temp = ( TlU8 * )tlMemory( NULL, R.conFontResX*R.conFontResY*2 );
		for( y = 0; y < R.conFontResY; ++y ) {
			for( x = 0; x < R.conFontResX; ++x ) {
				temp[ y*R.conFontResX*2 + x*2 + 0 ] = tl__g_devconFont[ y*R.conFontResX + x ];
				temp[ y*R.conFontResX*2 + x*2 + 1 ] = tl__g_devconFont[ y*R.conFontResX + x ];
			}
		}
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, R.conFontResX, R.conFontResY, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, ( void * )temp);
		tlMemory( ( void * )temp, 0 );
	}

#if SHADERS_ENABLED
	R.tileMap_vert = tlR_LoadGLSL( GL_VERTEX_SHADER, g_tileMap_vertSrc );
	R.tileMap_frag = tlR_LoadGLSL( GL_FRAGMENT_SHADER, g_tileMap_fragSrc );
	R.tileMap_prog = tlR_LinkGLSL( R.tileMap_vert, R.tileMap_frag );

	if( !R.tileMap_prog ) {
		exit( EXIT_FAILURE );
	}
#endif

	brush = tlNewBrush();

#if 0
	tlSetBrushCullMode(brush, kTlCM_Front);
	tlDisableBrushZTest(g_defaultBrush);
#endif
	tlEnableBrushRendering(brush);
	tlDisableBrushLighting(brush);

	g_defcam = tlNewCamera();
}
void tlR_Fini( void )
{
	if (!g_didInit)
		return;

	tlDeleteAllEntities();
	g_defcam = ( TlEntity * )0;
	g_didInit = FALSE;
}
TlEntity *tlR_DefaultCamera( void )
{
	return g_defcam;
}

/*
 * -------------------------------
 * TODO: Frustum-culling of sorts.
 * -------------------------------
 */
void tlR_DrawView(TlView *view) {
	GLbitfield clearBits;
	TlMat4 V;
	int vp[4];

	/* specify the viewport and the scissor rectangle */
	tlSetCameraEntity(view->ent);
	tlLoadAffineInverse(&V, tlGetEntityGlobalMatrix(view->ent));
	/*V = *tlGetEntityGlobalMatrix(view->ent);*/

	vp[0] = view->vpReal[0];
	vp[1] = view->vpReal[1];
	vp[2] = view->vpReal[2] - view->vpReal[0];
	vp[3] = view->vpReal[3] - view->vpReal[1];

	glViewport(vp[0], vp[1], vp[2], vp[3]);
	tlGL_CheckError();
	glDepthRange(0, 1);
	tlGL_CheckError();

	glEnable(GL_SCISSOR_TEST);
	tlGL_CheckError();
	glScissor(vp[0], vp[1], vp[2], vp[3]);
	tlGL_CheckError();

	/* perform the clear operation */
	clearBits = 0;
	clearBits |= view->clear.clearColor   ? GL_COLOR_BUFFER_BIT   : 0;
	clearBits |= view->clear.clearDepth   ? GL_DEPTH_BUFFER_BIT   : 0;
	clearBits |= view->clear.clearStencil ? GL_STENCIL_BUFFER_BIT : 0;

	if (clearBits) {

		/*R->ClearColor(view->clear.color[0], view->clear.color[1],
					  view->clear.color[2], view->clear.color[3]);*/
		glClearColor(view->clear.color[0], view->clear.color[1],
					 view->clear.color[2], view->clear.color[3]);
		tlGL_CheckError();
		/*R->ClearDepth(view->clear.depth);*/
		glClearDepth(view->clear.depth);
		tlGL_CheckError();
		/*R->ClearStencil(view->clear.stencil);*/
		glClearStencil(view->clear.stencil);
		tlGL_CheckError();

		/*R->Clear(clearBits);*/
		glClear(clearBits);
		tlGL_CheckError();

	}

	/* add the entities specified to the render queue */
	{
		TlEntity *ent;

		for(ent=tlFirstRootEntity(); ent!=(TlEntity *)0; ent=ent->next) {
			tlRQ_AddEntities(ent, &V);
		}
	}

	/* sort then draw the entities within the queue */
	tlRQ_Sort();
	tlRQ_Draw();
}
void tlR_Frame(double deltaTime) {
	static int lastw = 0, lasth = 0;
	static TlBool coversall = FALSE;
	TlView *view;
	int w, h;

	/*deltaTime will later be used for animations*/
	if(deltaTime){/*unused*/}

#if GLFW_ENABLED
	glfwGetFramebufferSize( tl__g_window, &w, &h );
#elif defined( _WIN32 )
	w = tlWin_ResX();
	h = tlWin_ResY();
#else
# error Not implemented
#endif

	if( w != lastw || h != lasth ) {
		/*printf( "Resized from %ix%i to %ix%i\n", lastw, lasth, w, h );*/

		lastw = w;
		lasth = h;

		/* determine whether any view covers the entire window AND recalculate viewports */
		coversall = FALSE;
		for(view=tlFirstView(); view!=(TlView *)0; view=view->next) {
			tlRecalcViewport(view, w, h);
			if( view->vpReal[0]<=0 && view->vpReal[1]<=0 && view->vpReal[2]>=w && view->vpReal[3]>=h ) {
				coversall = TRUE;
			}
		}

		/*printf( "Draw black bars? %s\n", coversall ? "No" : "Yes" );*/
	}

	/* clear the screen to black if there's no view that covers the whole screen */
	if( !coversall ) {
		glScissor( 0, 0, w, h );

		glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT );
	}

	/* draw each view */
	for(view=tlFirstView(); view!=(TlView *)0; view=view->next) {
		tlR_DrawView(view);
	}

	/* ### DEBUG DATA ### */
	#if 0
	glViewport( 0, 0, w, h );
	glDisable(GL_SCISSOR_TEST);
	{
		static int lastmmx = 0, lastmmy = 0;
		int mmx, mmy;
		char buf[ 512 ];

		mmx = tlMouseMoveX();
		mmy = tlMouseMoveY();

		if( mmx != 0 || mmy != 0 ) {
			lastmmx = mmx;
			lastmmy = mmy;
		} else {
			mmx = lastmmx;
			mmy = lastmmy;
		}

		sprintf( buf, "Mouse: %i, %i\nMouseMove: %i, %i", tlMouseX(), tlMouseY(), mmx, mmy );
		tlR_DrawText( buf, 5, 5, 300, 300 );
	}
	glDisable(GL_TEXTURE_2D);
	#endif

	/* get rid of unprocessed events */
	while( tlEv_Pending() ) {
		( void )tlEv_Next();
	}
	tlClearMouseMove();
	tlClearMouseWheel();

	/* sync */
#if GLFW_ENABLED
	glfwSwapBuffers( tl__g_window );
	glfwPollEvents();
#elif defined( _WIN32 )
	if( !tlWin_Loop() ) {
		return;
	}
	tlWin_SwapBuffers();
#endif
}

static void GetCharTexMap(float tc[4], TlS32 c)
{
	c -= 0x20;

	if (*(unsigned int *)&c >= 0x7F)
		c = 0; /*use blank space for non-printable characters*/

	tc[0] = ((float)((c%16)))*((float)R.conFontCellResX)/( float )R.conFontResX;
	tc[1] = ((float)((c/16)))*((float)R.conFontCellResY)/( float )R.conFontResY;
	tc[2] = tc[0] + ((float)R.conFontCellResX)/( float )R.conFontResX;
	tc[3] = tc[1] + ((float)R.conFontCellResY)/( float )R.conFontResY;
}

static void BindConsoleFont(void)
{
	/* set the texture */
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, R.conFontImage);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	/* overwrite the matrices to avoid weird rendering */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* disable stuff that shouldn't affect the text */
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	/* enable blending for translucency */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* enable alpha testing */
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.1f);
}

static void DrawChar(TlS32 x, TlS32 y, TlS32 charIndex, float screenResX, float screenResY)
{
	float tl[ 2 ], br[ 2 ];
	float tc[ 4 ];

	GetCharTexMap(tc, charIndex);

	tl[ 0 ] = ( float )x + 0.5f;
	tl[ 1 ] = ( float )y + 0.5f;
	br[ 0 ] = tl[ 0 ] + R.conFontCellResX;
	br[ 1 ] = tl[ 1 ] + R.conFontCellResY;

	tlScreenToProj( &tl[ 0 ], &tl[ 1 ], screenResX, screenResY );
	tlScreenToProj( &br[ 0 ], &br[ 1 ], screenResX, screenResY );

	glTexCoord2f(tc[0], tc[1]); glVertex2f(tl[0], tl[1]);
	glTexCoord2f(tc[2], tc[1]); glVertex2f(br[0], tl[1]);
	glTexCoord2f(tc[2], tc[3]); glVertex2f(br[0], br[1]);
	glTexCoord2f(tc[0], tc[3]); glVertex2f(tl[0], br[1]);
}

static void DrawTextOnly(const char *asciitext, TlS32 x, TlS32 y, TlU32 w, TlU32 h)
{
	const char *p;
	float swf, shf;
	int sw, sh;
	TlS32 basex, basey;
	TlS32 currx, curry;

	if( !asciitext || *asciitext == '\0' ) {
		return;
	}

#if GLFW_ENABLED
	glfwGetFramebufferSize( tl__g_window, &sw, &sh );
#elif defined( _WIN32 )
	sw = tlWin_ResX();
	sh = tlWin_ResY();
#else
# error Not implemented
#endif
	swf = ( float )sw;
	shf = ( float )sh;

	basex = x;
	basey = y;
	currx = x;
	curry = y;

	for( p = asciitext; *p != '\0'; ++p ) {
		if( *p == '\r' ) {
			currx = basex;
			continue;
		}
		if( *p == '\n' ) {
			currx = basex;
			curry += R.conFontCellResY;
			if( curry > ( TlS32 )( basey + h ) ) {
				break;
			}
			continue;
		}
		if( *p == '\t' ) {
			static const TlU32 ts = 4;
			TlU32 n;

			n = ( TlU32 )( currx - basex )/R.conFontCellResX;
			n += ts - n%ts;

			currx = basex + n*R.conFontCellResX;
			if( currx > basex + ( TlS32 )w ) {
				currx = basex;
				curry += R.conFontCellResY;
				if( curry > ( TlS32 )( basey + h ) ) {
					break;
				}
			}

			continue;
		}

		DrawChar( currx, curry, +*p, swf, shf );

		currx += R.conFontCellResX;
		if( currx + R.conFontCellResX > basex + w ) {
			currx = basex;
			curry += R.conFontCellResY;
			if( curry > ( TlS32 )( basey + h ) ) {
				break;
			}
		}
	}
}
void tlR_DrawText(const char *asciitext, TlS32 x, TlS32 y, TlU32 w, TlU32 h)
{
	BindConsoleFont();
	glBegin(GL_QUADS);
	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	DrawTextOnly(asciitext, x, y, w, h);
	glEnd();
}

GLuint tlR_LoadGLSL( GLuint shaderType, const char *pszSourceCode )
{
	GLuint shader;
	GLint status;

	shader = R.CreateShader( shaderType );
	if( !shader ) {
		return 0;
	}

	R.ShaderSource( shader, 1, &pszSourceCode, NULL );
	R.CompileShader( shader );

	R.GetShaderiv( shader, GL_COMPILE_STATUS, &status );
	if( !status ) {
		char buf[ 4096 ];
		GLsizei length;

		R.GetShaderInfoLog( shader, sizeof( buf ), &length, buf );
		R.DeleteShader( shader );

		if( length < ( GLsizei )sizeof( buf ) && length >= 0 ) {
			buf[ length ] = '\0';
		} else {
			buf[ sizeof( buf ) - 1 ] = '\0';
		}

		tlErrorMessage( "LoadGLSL tlError: %s\n", buf );
		return 0;
	}

	return shader;
}
GLuint tlR_LinkGLSL( GLuint vertShader, GLuint fragShader )
{
	GLuint program;
	GLint status;

	program = R.CreateProgram();

	R.AttachShader( program, vertShader );
	R.AttachShader( program, fragShader );

	R.BindAttribLocation( program, 0, "vPosition" );
	R.BindAttribLocation( program, 1, "vTexCoord" );

	R.LinkProgram( program );

	R.GetProgramiv( program, GL_LINK_STATUS, &status );
	if( !status ) {
		char buf[ 4096 ];
		GLsizei length;

		R.GetProgramInfoLog( program, sizeof( buf ), &length, buf );
		R.DeleteProgram( program );

		if( length < ( GLsizei )sizeof( buf ) && length >= 0 ) {
			buf[ length ] = '\0';
		} else {
			buf[ sizeof( buf ) - 1 ] = '\0';
		}

		tlErrorMessage( "LinkGLSL tlError: %s\n", buf );
		return 0;
	}

	return program;
}

GLuint tlR_CreateShader(GLenum shaderType) {
	return R.CreateShader(shaderType);
}
void tlR_ShaderSource(GLuint shader, int numStrings, const char **strings,
int *stringLengths) {
	R.ShaderSource(shader, numStrings, strings, stringLengths);
}
void tlR_CompileShader(GLuint shader) {
	R.CompileShader(shader);
}
GLuint tlR_CreateProgram() {
	return R.CreateProgram();
}
void tlR_AttachObject(GLuint program, GLuint shader) {
	R.AttachShader(program, shader);
}
void tlR_LinkProgram(GLuint program) {
	R.LinkProgram(program);
}
void tlR_UseProgram(GLuint program) {
	R.UseProgram(program);
}

void tlR_GetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length,
char *infoLog) {
	R.GetShaderInfoLog(shader, bufSize, length, infoLog);
}
void tlR_GetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length,
char *infoLog) {
	R.GetProgramInfoLog(program, bufSize, length, infoLog);
}

void tlR_DeleteShader(GLuint shader) {
	R.DeleteShader(shader);
}
void tlR_DeleteProgram(GLuint program) {
	R.DeleteProgram(program);
}

