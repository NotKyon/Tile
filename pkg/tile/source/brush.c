#include <tile/brush.h>
#include <tile/renderer.h>

/*
 * ==========================================================================
 *
 *	BRUSH SYSTEM
 *
 * ==========================================================================
 */
static TlBrush *g_brush_head = (TlBrush *)0;
static TlBrush *g_brush_tail = (TlBrush *)0;

TlBrush *tlNewBrush() {
	TlBrush *brush;

	brush = tlAllocStruct(TlBrush);

	brush->refCnt = 0;

	brush->shader.vert = 0;
	brush->shader.frag = 0;
	brush->shader.prog = 0;

	brush->lighting.diffuse.r = 1.0f;
	brush->lighting.diffuse.g = 1.0f;
	brush->lighting.diffuse.b = 1.0f;
	brush->lighting.diffuse.a = 1.0f;
	brush->lighting.ambient.r = 0.0f;
	brush->lighting.ambient.g = 0.0f;
	brush->lighting.ambient.b = 0.0f;
	brush->lighting.ambient.a = 0.0f;
	brush->lighting.emissive.r = 0.0f;
	brush->lighting.emissive.g = 0.0f;
	brush->lighting.emissive.b = 0.0f;
	brush->lighting.emissive.a = 0.0f;
	brush->lighting.specular.r = 0.0f;
	brush->lighting.specular.g = 0.0f;
	brush->lighting.specular.b = 0.0f;
	brush->lighting.specular.a = 0.0f;
	brush->lighting.shininess = 0.0f;

	brush->lighting.isLit = TRUE;

	brush->lighting.useDiffuse = TRUE;
	brush->lighting.useAmbient = TRUE;
	brush->lighting.useEmissive = TRUE;
	brush->lighting.useSpecular = TRUE;

	brush->drawing.isVisible = TRUE;
	brush->drawing.usesBinorm = FALSE;
	brush->drawing.usesTangent = FALSE;
	brush->drawing.cullMode = kTlCM_Back;
	brush->drawing.zCmpFunc = kTlCF_LessEqual;
	brush->drawing.zTest = TRUE;
	brush->drawing.zWrite = TRUE;
	brush->drawing.zSort = FALSE;

	brush->next = (TlBrush *)0;
	if ((brush->prev = g_brush_tail) != (TlBrush *)0)
		g_brush_tail->next = brush;
	else
		g_brush_head = brush;
	g_brush_tail = brush;

	return brush;
}
TlBrush *tlCopyBrush(const TlBrush *copyFrom) {
	TlBrush *brush;

	brush = tlNewBrush();

	brush->shader.vert = 0;
	brush->shader.frag = 0;
	brush->shader.prog = 0;

	brush->lighting.diffuse.r = copyFrom->lighting.diffuse.r;
	brush->lighting.diffuse.g = copyFrom->lighting.diffuse.g;
	brush->lighting.diffuse.b = copyFrom->lighting.diffuse.b;
	brush->lighting.diffuse.a = copyFrom->lighting.diffuse.a;
	brush->lighting.ambient.r = copyFrom->lighting.ambient.r;
	brush->lighting.ambient.g = copyFrom->lighting.ambient.g;
	brush->lighting.ambient.b = copyFrom->lighting.ambient.b;
	brush->lighting.ambient.a = copyFrom->lighting.ambient.a;
	brush->lighting.emissive.r = copyFrom->lighting.emissive.r;
	brush->lighting.emissive.g = copyFrom->lighting.emissive.g;
	brush->lighting.emissive.b = copyFrom->lighting.emissive.b;
	brush->lighting.emissive.a = copyFrom->lighting.emissive.a;
	brush->lighting.specular.r = copyFrom->lighting.specular.r;
	brush->lighting.specular.g = copyFrom->lighting.specular.g;
	brush->lighting.specular.b = copyFrom->lighting.specular.b;
	brush->lighting.specular.a = copyFrom->lighting.specular.a;
	brush->lighting.shininess = copyFrom->lighting.shininess;

	brush->lighting.isLit = copyFrom->lighting.isLit;

	brush->lighting.useDiffuse = copyFrom->lighting.useDiffuse;
	brush->lighting.useAmbient = copyFrom->lighting.useAmbient;
	brush->lighting.useEmissive = copyFrom->lighting.useEmissive;
	brush->lighting.useSpecular = copyFrom->lighting.useSpecular;

	brush->drawing.isVisible = copyFrom->drawing.isVisible;
	brush->drawing.usesBinorm = copyFrom->drawing.usesBinorm;
	brush->drawing.usesTangent = copyFrom->drawing.usesTangent;
	brush->drawing.cullMode = copyFrom->drawing.cullMode;
	brush->drawing.zCmpFunc = copyFrom->drawing.zCmpFunc;
	brush->drawing.zTest = copyFrom->drawing.zTest;
	brush->drawing.zWrite = copyFrom->drawing.zWrite;
	brush->drawing.zSort = copyFrom->drawing.zSort;

	return brush;
}
TlBrush *tlDeleteBrush(TlBrush *brush) {
	if (!brush)
		return (TlBrush *)0;

	if (--brush->refCnt > 0)
		return (TlBrush *)0;

	if (brush->shader.prog) {
		tlR_DeleteProgram(brush->shader.prog);
		brush->shader.prog = 0;
	}
	if (brush->shader.frag) {
		tlR_DeleteShader(brush->shader.frag);
		brush->shader.frag = 0;
	}
	if (brush->shader.vert) {
		tlR_DeleteShader(brush->shader.vert);
		brush->shader.vert = 0;
	}

	if (brush->prev)
		brush->prev->next = brush->next;
	if (brush->next)
		brush->next->prev = brush->prev;

	if (g_brush_head==brush)
		g_brush_head = brush->next;
	if (g_brush_tail==brush)
		g_brush_tail = brush->prev;

	return (TlBrush *)tlMemory((void *)brush, 0);
}

void tlRetainBrush(TlBrush *brush) {
	brush->refCnt++;
}

void tlSetBrushShader(TlBrush *brush, const char *vertSrc, const char *fragSrc) {
	char infoLog[8192];

	if (brush->shader.prog) {
		tlR_DeleteProgram(brush->shader.prog);
		brush->shader.prog = 0;
	}
	if (brush->shader.frag) {
		tlR_DeleteShader(brush->shader.frag);
		brush->shader.frag = 0;
	}
	if (brush->shader.vert) {
		tlR_DeleteShader(brush->shader.vert);
		brush->shader.vert = 0;
	}

	if (vertSrc) brush->shader.vert = tlR_CreateShader(GL_VERTEX_SHADER_ARB);
	if (fragSrc) brush->shader.frag = tlR_CreateShader(GL_FRAGMENT_SHADER_ARB);

	if (!vertSrc && !fragSrc)
		return;

	/*
	 * TODO: Better error reporting...
	 */

	brush->shader.prog = tlR_CreateProgram();

	if (vertSrc) {
		tlR_ShaderSource(brush->shader.vert, 1, &vertSrc, (int *)0);
		tlR_CompileShader(brush->shader.vert);

		tlR_GetShaderInfoLog(brush->shader.vert, sizeof(infoLog), 0, infoLog);
		fprintf(stderr, "%s\n", infoLog);

		tlR_AttachObject(brush->shader.prog, brush->shader.vert);
	}

	if (fragSrc) {
		tlR_ShaderSource(brush->shader.frag, 1, &fragSrc, (int *)0);
		tlR_CompileShader(brush->shader.frag);

		tlR_GetShaderInfoLog(brush->shader.frag, sizeof(infoLog), 0, infoLog);
		fprintf(stderr, "%s\n", infoLog);

		tlR_AttachObject(brush->shader.prog, brush->shader.frag);
	}

	tlR_LinkProgram(brush->shader.prog);

	tlR_GetProgramInfoLog(brush->shader.prog, sizeof(infoLog), 0, infoLog);
	fprintf(stderr, "%s\n", infoLog);
}
TlBool tlIsBrushVertexShaderAttached(const TlBrush *brush) {
	return brush->shader.vert ? TRUE : FALSE;
}
TlBool tlIsBrushFragmentShaderAttached(const TlBrush *brush) {
	return brush->shader.frag ? TRUE : FALSE;
}
TlBool tlIsBrushShaderAttached(const TlBrush *brush) {
	return brush->shader.prog ? TRUE : FALSE;
}

void tlSetBrushDiffuse(TlBrush *brush, float r, float g, float b, float a) {
	brush->lighting.diffuse.r = r;
	brush->lighting.diffuse.g = g;
	brush->lighting.diffuse.b = b;
	brush->lighting.diffuse.a = a;
}
float tlGetBrushDiffuseR(const TlBrush *brush) {
	return brush->lighting.diffuse.r;
}
float tlGetBrushDiffuseG(const TlBrush *brush) {
	return brush->lighting.diffuse.g;
}
float tlGetBrushDiffuseB(const TlBrush *brush) {
	return brush->lighting.diffuse.b;
}
float tlGetBrushDiffuseA(const TlBrush *brush) {
	return brush->lighting.diffuse.a;
}

void tlSetBrushAmbient(TlBrush *brush, float r, float g, float b, float a) {
	brush->lighting.ambient.r = r;
	brush->lighting.ambient.g = g;
	brush->lighting.ambient.b = b;
	brush->lighting.ambient.a = a;
}
float tlGetBrushAmbientR(const TlBrush *brush) {
	return brush->lighting.ambient.r;
}
float tlGetBrushAmbientG(const TlBrush *brush) {
	return brush->lighting.ambient.g;
}
float tlGetBrushAmbientB(const TlBrush *brush) {
	return brush->lighting.ambient.b;
}
float tlGetBrushAmbientA(const TlBrush *brush) {
	return brush->lighting.ambient.a;
}

void tlSetBrushEmissive(TlBrush *brush, float r, float g, float b, float a) {
	brush->lighting.emissive.r = r;
	brush->lighting.emissive.g = g;
	brush->lighting.emissive.b = b;
	brush->lighting.emissive.a = a;
}
float tlGetBrushEmissiveR(const TlBrush *brush) {
	return brush->lighting.emissive.r;
}
float tlGetBrushEmissiveG(const TlBrush *brush) {
	return brush->lighting.emissive.g;
}
float tlGetBrushEmissiveB(const TlBrush *brush) {
	return brush->lighting.emissive.b;
}
float tlGetBrushEmissiveA(const TlBrush *brush) {
	return brush->lighting.emissive.a;
}

void tlSetBrushSpecular(TlBrush *brush, float r, float g, float b, float a) {
	brush->lighting.specular.r = r;
	brush->lighting.specular.g = g;
	brush->lighting.specular.b = b;
	brush->lighting.specular.a = a;
}
float tlGetBrushSpecularR(const TlBrush *brush) {
	return brush->lighting.specular.r;
}
float tlGetBrushSpecularG(const TlBrush *brush) {
	return brush->lighting.specular.g;
}
float tlGetBrushSpecularB(const TlBrush *brush) {
	return brush->lighting.specular.b;
}
float tlGetBrushSpecularA(const TlBrush *brush) {
	return brush->lighting.specular.a;
}

void tlSetBrushShininess(TlBrush *brush, float shininess) {
	brush->lighting.shininess = shininess;
}
float tlGetBrushShininess(const TlBrush *brush) {
	return brush->lighting.shininess;
}

void tlEnableBrushLighting(TlBrush *brush) {
	brush->lighting.isLit = TRUE;
}
void tlDisableBrushLighting(TlBrush *brush) {
	brush->lighting.isLit = FALSE;
}
TlBool tlIsBrushLightingEnabled(const TlBrush *brush) {
	return brush->lighting.isLit;
}

void tlEnableBrushDiffuseLighting(TlBrush *brush) {
	brush->lighting.useDiffuse = TRUE;
}
void tlDisableBrushDiffuseLighting(TlBrush *brush) {
	brush->lighting.useDiffuse = FALSE;
}
TlBool tlIsBrushDiffuseLightingEnabled(const TlBrush *brush) {
	return brush->lighting.useDiffuse;
}

void tlEnableBrushAmbientLighting(TlBrush *brush) {
	brush->lighting.useAmbient = TRUE;
}
void tlDisableBrushAmbientLighting(TlBrush *brush) {
	brush->lighting.useAmbient = FALSE;
}
TlBool tlIsBrushAmbientLightingEnabled(const TlBrush *brush) {
	return brush->lighting.useAmbient;
}

void tlEnableBrushEmissiveLighting(TlBrush *brush) {
	brush->lighting.useEmissive = TRUE;
}
void tlDisableBrushEmissiveLighting(TlBrush *brush) {
	brush->lighting.useEmissive = FALSE;
}
TlBool tlIsBrushEmissiveLightingEnabled(const TlBrush *brush) {
	return brush->lighting.useEmissive;
}

void tlEnableBrushSpecularLighting(TlBrush *brush) {
	brush->lighting.useSpecular = TRUE;
}
void tlDisableBrushSpecularLighting(TlBrush *brush) {
	brush->lighting.useSpecular = FALSE;
}
TlBool tlIsBrushSpecularLightingEnabled(const TlBrush *brush) {
	return brush->lighting.useSpecular;
}

void tlEnableBrushRendering(TlBrush *brush) {
	brush->drawing.isVisible = TRUE;
}
void tlDisableBrushRendering(TlBrush *brush) {
	brush->drawing.isVisible = FALSE;
}
TlBool tlIsBrushRenderingEnabled(const TlBrush *brush) {
	return brush->drawing.isVisible;
}

void tlEnableBrushBinormal(TlBrush *brush) {
	brush->drawing.usesBinorm = TRUE;
}
void tlDisableBrushBinormal(TlBrush *brush) {
	brush->drawing.usesBinorm = FALSE;
}
TlBool tlIsBrushBinormalEnabled(const TlBrush *brush) {
	return brush->drawing.usesBinorm;
}

void tlEnableBrushTangent(TlBrush *brush) {
	brush->drawing.usesTangent = TRUE;
}
void tlDisableBrushTangent(TlBrush *brush) {
	brush->drawing.usesTangent = FALSE;
}
TlBool tlIsBrushTangentEnabled(const TlBrush *brush) {
	return brush->drawing.usesTangent;
}

void tlSetBrushCullMode(TlBrush *brush, TlCullMode_t cullMode) {
	brush->drawing.cullMode = cullMode;
}
TlCullMode_t tlGetBrushCullMode(const TlBrush *brush) {
	return brush->drawing.cullMode;
}

void tlSetBrushZFunc(TlBrush *brush, TlCmpFunc_t zCmpFunc) {
	brush->drawing.zCmpFunc = zCmpFunc;
}
TlCmpFunc_t tlGetBrushZFunc(const TlBrush *brush) {
	return brush->drawing.zCmpFunc;
}

void tlEnableBrushZTest(TlBrush *brush) {
	brush->drawing.zTest = TRUE;
}
void tlDisableBrushZTest(TlBrush *brush) {
	brush->drawing.zTest = FALSE;
}
TlBool tlIsBrushZTestEnabled(const TlBrush *brush) {
	return brush->drawing.zTest;
}

void tlEnableBrushZWrite(TlBrush *brush) {
	brush->drawing.zWrite = TRUE;
}
void tlDisableBrushZWrite(TlBrush *brush) {
	brush->drawing.zWrite = FALSE;
}
TlBool tlIsBrushZWriteEnabled(const TlBrush *brush) {
	return brush->drawing.zWrite;
}

void tlEnableBrushZSort(TlBrush *brush) {
	brush->drawing.zSort = TRUE;
}
void tlDisableBrushZSort(TlBrush *brush) {
	brush->drawing.zSort = FALSE;
}
TlBool tlIsBrushZSortEnabled(const TlBrush *brush) {
	return brush->drawing.zSort;
}

TlBrush *tlBrushBefore(const TlBrush *brush) {
	return brush->prev;
}
TlBrush *tlBrushAfter(const TlBrush *brush) {
	return brush->next;
}
TlBrush *tlFirstBrush() {
	return g_brush_head;
}
TlBrush *tlLastBrush() {
	return g_brush_tail;
}

