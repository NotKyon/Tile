#ifndef TILE_BRUSH_H
#define TILE_BRUSH_H

#include "const.h"
#include "math.h"

TILE_EXTRNC_ENTER

/*
 * ----
 * Culling Mode
 * ----
 * The culling mode tells the graphics card when *not* to draw polygons,
 * depending on which way they're facing.
 *
 * The default is `kTlCM_Back`, which means polygons will not be rendered if
 * they're not facing the camera. Imagine a cube, for example. There's a camera
 * in front of the cube. So, the graphics card gets all six sides of the cube
 * sent to it, but we can only see three at most. (Possibly only one!) It would
 * be wasteful for the graphics card to process all six sides even though we're
 * only able to see three sides. With "backface culling" (kTlCM_Back, the
 * default) enabled the GPU will, upon determining which way the other three
 * polygons are facing, stop trying to render them.
 *
 * What if you wanted to see inside of the cube though? This might be the case
 * if you want to display the inside of a building with the camera backed up.
 * (Common in several older 3D RPGs.) You can set the culling mode to
 * `kTlCM_Front` instead. This will cause the graphics card to ignore all
 * polygons that are facing the camera (with this culling mode), but keep all
 * the polygons that are facing away from the camera.
 */
typedef enum {
	/* Culling will be completely disabled */
	kTlCM_None,
	/* Cull front facing polygons (you can see the "inside") */
	kTlCM_Front,
	/* Cull back facing polygons (default; you see the outside) */
	kTlCM_Back,
	/* Everything will be culled (surfaces will be invisible) FIXME: Explain when this is useful */
	kTlCM_Both
} TlCullMode_t;
/*
 * ----
 * Comparison Function
 * ----
 * Comparison functions are used by certain parts of the graphics pipeline to
 * compare two values. The common case will be a depth-test, for example.
 */
typedef enum {
	/* Comparison never succeeds; always returns false. */
	kTlCF_Never,
	/* Comparison is true if a < b. */
	kTlCF_Less,
	/* Comparison is true if a <= b. */
	kTlCF_LessEqual,
	/* Comparison is true if a == b. */
	kTlCF_Equal,
	/* Comparison is true if a != b. */
	kTlCF_NotEqual,
	/* Comparison is true if a >= b. */
	kTlCF_GreaterEqual,
	/* Comparison is true if a > b. */
	kTlCF_Greater,
	/* Comparison always succeeds; always returns true. */
	kTlCF_Always
} TlCmpFunc_t;

/*
 * -----
 * Brush
 * -----
 * Brushes are used to define the appearance of a given Surface.
 */
typedef struct TL_CACHELINE_ALIGNED TlBrush_s {
	int refCnt;

	struct {
		GLuint vert, frag;
		GLuint prog;
	} shader;

	struct {
		TlColor diffuse, ambient, emissive, specular;
		float shininess;

		TlBool isLit:1;

		TlBool useDiffuse:1;
		TlBool useAmbient:1;
		TlBool useEmissive:1;
		TlBool useSpecular:1;
	} lighting;

	struct {
		TlBool isVisible:1;
		TlBool usesBinorm:1;
		TlBool usesTangent:1;
		/*
		TlCullMode_t cullMode:2;
		TlCmpFunc_t zCmpFunc:3;
		*/
		unsigned cullMode:2;
		unsigned zCmpFunc:3;
		TlBool zTest:1;
		TlBool zWrite:1;
		TlBool zSort:1;
	} drawing;

	struct TlBrush_s *prev, *next;
} TlBrush;

/* Create a new brush. */
TlBrush *tlNewBrush();
/* Create a new brush, copying its settings from another brush. */
TlBrush *tlCopyBrush(const TlBrush *copyFrom);
/* Delete a brush. (The reference count is decremented. If it's zero the brush is actually deleted.) */
TlBrush *tlDeleteBrush(TlBrush *brush);

/* Increment the reference count of a brush. This prevents delete from immediately destroying it. */
void tlRetainBrush(TlBrush *brush);

/* Set the vertex and fragment shaders of a brush. */
void tlSetBrushShader(TlBrush *brush, const char *vertSrc, const char *fragSrc);
/* Check whether a vertex shader is attached to a given brush. */
TlBool tlIsBrushVertexShaderAttached(const TlBrush *brush);
/* Check whether a fragment shader is attached to a given brush. */
TlBool tlIsBrushFragmentShaderAttached(const TlBrush *brush);
/* Check whether any shader is attached to a given brush. */
TlBool tlIsBrushShaderAttached(const TlBrush *brush);

/* Set the diffuse (color) material of the given brush. */
void tlSetBrushDiffuse(TlBrush *brush, float r, float g, float b, float a);
/* Retrieve the given brush's diffuse (color) material's red channel. */
float tlGetBrushDiffuseR(const TlBrush *brush);
/* Retrieve the given brush's diffuse (color) material's green channel. */
float tlGetBrushDiffuseG(const TlBrush *brush);
/* Retrieve the given brush's diffuse (color) material's blue channel. */
float tlGetBrushDiffuseB(const TlBrush *brush);
/* Retrieve the given brush's diffuse (color) material's alpha channel. */
float tlGetBrushDiffuseA(const TlBrush *brush);

/* Set the ambient material of the given brush. */
void tlSetBrushAmbient(TlBrush *brush, float r, float g, float b, float a);
/* Retrieve the given brush's ambient material's red channel. */
float tlGetBrushAmbientR(const TlBrush *brush);
/* Retrieve the given brush's ambient material's green channel. */
float tlGetBrushAmbientG(const TlBrush *brush);
/* Retrieve the given brush's ambient material's blue channel. */
float tlGetBrushAmbientB(const TlBrush *brush);
/* Retrieve the given brush's ambient material's alpha channel. */
float tlGetBrushAmbientA(const TlBrush *brush);

/* Set the emissive material of the given brush. */
void tlSetBrushEmissive(TlBrush *brush, float r, float g, float b, float a);
/* Retrieve the given brush's emissive material's red channel. */
float tlGetBrushEmissiveR(const TlBrush *brush);
/* Retrieve the given brush's emissive material's green channel. */
float tlGetBrushEmissiveG(const TlBrush *brush);
/* Retrieve the given brush's emissive material's blue channel. */
float tlGetBrushEmissiveB(const TlBrush *brush);
/* Retrieve the given brush's emissive material's alpha channel. */
float tlGetBrushEmissiveA(const TlBrush *brush);

/* Set the specular material of the given brush. */
void tlSetBrushSpecular(TlBrush *brush, float r, float g, float b, float a);
/* Retrieve the given brush's specular material's red channel. */
float tlGetBrushSpecularR(const TlBrush *brush);
/* Retrieve the given brush's specular material's green channel. */
float tlGetBrushSpecularG(const TlBrush *brush);
/* Retrieve the given brush's specular material's blue channel. */
float tlGetBrushSpecularB(const TlBrush *brush);
/* Retrieve the given brush's specular material's alpha channel. */
float tlGetBrushSpecularA(const TlBrush *brush);

/* Set the shininess of the specular material of the given brush. */
void tlSetBrushShininess(TlBrush *brush, float shininess);
/* Retrieve the shininess of the specular material of the given brush. */
float tlGetBrushShininess(const TlBrush *brush);

/* Ensure the given brush responds to lighting (or the lack thereof). */
void tlEnableBrushLighting(TlBrush *brush);
/* Prevent the given brush from responding to lighting (or the lack thereof). */
void tlDisableBrushLighting(TlBrush *brush);
/* Retrieve whether lighting affects the brush. */
TlBool tlIsBrushLightingEnabled(const TlBrush *brush);

/* Ensure the diffuse material of the brush can be used in lighting calculations. */
void tlEnableBrushDiffuseLighting(TlBrush *brush);
/* Prevent the diffuse material of the brush from being used in lighting calculations. */
void tlDisableBrushDiffuseLighting(TlBrush *brush);
/* Retrieve whether the diffuse material of the brush can be used in lighting calculations. */
TlBool tlIsBrushDiffuseLightingEnabled(const TlBrush *brush);

/* Ensure the ambient material of the brush can be used in lighting calculations. */
void tlEnableBrushAmbientLighting(TlBrush *brush);
/* Prevent the ambient material of the brush from being used in lighting calculations. */
void tlDisableBrushAmbientLighting(TlBrush *brush);
/* Retrieve whether the ambient material of the brush can be used in lighting calculations. */
TlBool tlIsBrushAmbientLightingEnabled(const TlBrush *brush);

/* Ensure the emissive material of the brush can be used in lighting calculations. */
void tlEnableBrushEmissiveLighting(TlBrush *brush);
/* Prevent the emissive material of the brush from being used in lighting calculations. */
void tlDisableBrushEmissiveLighting(TlBrush *brush);
/* Retrieve whether the emissive material of the brush can be used in lighting calculations. */
TlBool tlIsBrushEmissiveLightingEnabled(const TlBrush *brush);

/* Ensure the specular material of the brush can be used in lighting calculations. */
void tlEnableBrushSpecularLighting(TlBrush *brush);
/* Prevent the specular material of the brush from being used in lighting calculations. */
void tlDisableBrushSpecularLighting(TlBrush *brush);
/* Retrieve whether the specular material of the brush can be used in lighting calculations. */
TlBool tlIsBrushSpecularLightingEnabled(const TlBrush *brush);

/* Ensure the given brush can be used for rendering. (If not, everything with this brush will be invisible.) */
void tlEnableBrushRendering(TlBrush *brush);
/* Prevent the given brush from being used for rendering. */
void tlDisableBrushRendering(TlBrush *brush);
/* Retrieve whether the given brush can be used for rendering. (If not, everything with this brush will be invisible.) */
TlBool tlIsBrushRenderingEnabled(const TlBrush *brush);

/* Enable the use of binormals for per-fragment lighting on the given brush. */
void tlEnableBrushBinormal(TlBrush *brush);
/* Disable the use of binormals for per-fragment lighting on the given brush. */
void tlDisableBrushBinormal(TlBrush *brush);
/* Retrieve whether the use of binormals is enabled on the given brush. */
TlBool tlIsBrushBinormalEnabled(const TlBrush *brush);

/* Enable the use of tangents for per-fragment lighting on the given brush. */
void tlEnableBrushTangent(TlBrush *brush);
/* Disable the use of tangents for per-fragment lighting on the given brush. */
void tlDisableBrushTangent(TlBrush *brush);
/* Retrieve whether the use of tangents is enabled on the given brush. */
TlBool tlIsBrushTangentEnabled(const TlBrush *brush);

/* Set the culling mode for the given brush. */
void tlSetBrushCullMode(TlBrush *brush, TlCullMode_t cullMode);
/* Retrieve the culling mode for the given brush. */
TlCullMode_t tlGetBrushCullMode(const TlBrush *brush);

/* Set the depth comparison function for the given brush. */
void tlSetBrushZFunc(TlBrush *brush, TlCmpFunc_t zCmpFunc);
/* Retrieve the depth comparison function for the given brush. */
TlCmpFunc_t tlGetBrushZFunc(const TlBrush *brush);

/* Enable depth-testing for the given brush. */
void tlEnableBrushZTest(TlBrush *brush);
/* Disables depth-testing for the given brush. */
void tlDisableBrushZTest(TlBrush *brush);
/* Retrieve whether depth-testing is enabled for the given brush. */
TlBool tlIsBrushZTestEnabled(const TlBrush *brush);

/* Enable depth-writing for the given brush. */
void tlEnableBrushZWrite(TlBrush *brush);
/* Disable depth-writing for the given brush. */
void tlDisableBrushZWrite(TlBrush *brush);
/* Retrieve whether depth-writing is enabled for the given brush. */
TlBool tlIsBrushZWriteEnabled(const TlBrush *brush);

/* Enable depth sorting (a CPU-side feature) for the given brush. */
void tlEnableBrushZSort(TlBrush *brush);
/* Disable depth sorting (a CPU-side feature) for the given brush. */
void tlDisableBrushZSort(TlBrush *brush);
/* Retrieve whether depth sorting (a CPU-side feature) for the given brush is enabled. */
TlBool tlIsBrushZSortEnabled(const TlBrush *brush);

/* Retrieve the global brush resource that comes before the given brush. (Older.) */
TlBrush *tlBrushBefore(const TlBrush *brush);
/* Retrieve the global brush resource that comes after the given brush. (Newer.) */
TlBrush *tlBrushAfter(const TlBrush *brush);
/* Retrieve the first global brush resource. (Oldest.) */
TlBrush *tlFirstBrush();
/* Retrieve the last global brush resource. (Newest.) */
TlBrush *tlLastBrush();

TILE_EXTRNC_LEAVE

#endif

