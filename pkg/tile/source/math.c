#include <tile/math.h>

/*
	M[ 0]=xx; M[ 4]=xy; M[ 8]=xz; M[12]=xw;
	M[ 1]=yx; M[ 5]=yy; M[ 9]=yz; M[13]=yw;
	M[ 2]=zx; M[ 6]=zy; M[10]=zz; M[14]=zw;
	M[ 3]=wx; M[ 7]=wy; M[11]=wz; M[15]=ww;
*/

#ifndef TILE_REVERSE_ROTATION_ENABLED
# define TILE_REVERSE_ROTATION_ENABLED 1
#endif

static void Math_LoadIdentity(float *M)
{
	M[ 0]=1; M[ 4]=0; M[ 8]=0; M[12]=0;
	M[ 1]=0; M[ 5]=1; M[ 9]=0; M[13]=0;
	M[ 2]=0; M[ 6]=0; M[10]=1; M[14]=0;
	M[ 3]=0; M[ 7]=0; M[11]=0; M[15]=1;
}
static void Math_LoadPerspective(float *M, float fov, float aspect, float zn, float zf)
{
	float a, b, c, d;

	b = 1.0f/tlTan(0.5f*fov);
	a = b/aspect;

	c =     zf/(zf - zn);
	d = -zn*zf/(zf - zn);

	M[ 0]=a; M[ 4]=0; M[ 8]=0; M[12]=0;
	M[ 1]=0; M[ 5]=b; M[ 9]=0; M[13]=0;
	M[ 2]=0; M[ 6]=0; M[10]=c; M[14]=d;
	M[ 3]=0; M[ 7]=0; M[11]=1; M[15]=0;
}
static void Math_LoadOrtho(float *M, float l, float r, float b, float t, float zn, float zf)
{
	float A, B, C, D, E, F;

	A =    2.0f/(r  -  l);
	B = (l + r)/(l  -  r);
	C =    2.0f/(t  -  b);
	D = (t + b)/(b  -  t);
	E =    1.0f/(zf - zn);
	F =      zn/(zn - zf);

	M[ 0]=A; M[ 4]=0; M[ 8]=0; M[12]=B;
	M[ 1]=0; M[ 5]=C; M[ 9]=0; M[13]=D;
	M[ 2]=0; M[ 6]=0; M[10]=E; M[14]=F;
	M[ 3]=0; M[ 7]=0; M[11]=0; M[15]=1;
}
static void Math_LoadTranslation(float *M, float x, float y, float z)
{
	M[ 0]=1; M[ 4]=0; M[ 8]=0; M[12]=x;
	M[ 1]=0; M[ 5]=1; M[ 9]=0; M[13]=y;
	M[ 2]=0; M[ 6]=0; M[10]=1; M[14]=z;
	M[ 3]=0; M[ 7]=0; M[11]=0; M[15]=1;
}
static void Math_LoadRotation(float *M, float x, float y, float z)
{
	float sx,sy,sz, cx,cy,cz;
	float cz_nsx;

	sx=tlSin(x); sy=tlSin(y); sz=tlSin(z);
	cx=tlCos(x); cy=tlCos(y); cz=tlCos(z);

	cz_nsx = cz*-sx;

#if TILE_REVERSE_ROTATION_ENABLED
	M[ 0]= cz*cy;              M[ 4]=-sz*cx;  M[ 8]= cz*-sy + (sz*sx)*cy;
	M[ 1]= sz*cy + cz_nsx*sy;  M[ 5]= cz*cx;  M[ 9]= sz*-sy + cz_nsx*cy;
	M[ 2]= cx*sy;              M[ 6]= sx;     M[10]= cx*cy;
#else
	M[ 0]= cz*cy;              M[ 1]=-sz*cx;  M[ 2]= cz*-sy + (sz*sx)*cy;
	M[ 4]= sz*cy + cz_nsx*sy;  M[ 5]= cz*cx;  M[ 6]= sz*-sy + cz_nsx*cy;
	M[ 8]= cx*sy;              M[ 9]= sx;     M[10]= cx*cy;
#endif

	                           M[12]=0;
	                           M[13]=0;
	                           M[14]=0;
	M[ 3]=0; M[ 7]=0; M[11]=0; M[15]=1;
}
static void Math_LoadScaling(float *M, float x, float y, float z)
{
	M[ 0]=x; M[ 4]=0; M[ 8]=0; M[12]=0;
	M[ 1]=0; M[ 5]=y; M[ 9]=0; M[13]=0;
	M[ 2]=0; M[ 6]=0; M[10]=z; M[14]=0;
	M[ 3]=0; M[ 7]=0; M[11]=0; M[15]=1;
}

static void Math_ApplyTranslation(float *M, float x, float y, float z)
{
	M[12] += M[ 0]*x + M[ 4]*y + M[ 8]*z;
	M[13] += M[ 1]*x + M[ 5]*y + M[ 9]*z;
	M[14] += M[ 2]*x + M[ 6]*y + M[10]*z;
}
static void Math_tlApplyXRotation(float *M, float x)
{
	float c, s, t;

	c = tlCos(x);
	s = tlSin(x);

#if TILE_REVERSE_ROTATION_ENABLED
	t = M[ 4];
	M[ 4] = t*c  + M[ 8]*s;
	M[ 8] = t*-s + M[ 8]*c;

	t = M[ 5];
	M[ 5] = t*c  + M[ 9]*s;
	M[ 9] = t*-s + M[ 9]*c;

	t = M[ 6];
	M[ 6] = t*c  + M[10]*s;
	M[10] = t*-s + M[10]*c;
#else
	t = M[ 1];
	M[ 1] = t*c  + M[ 2]*s;
	M[ 2] = t*-s + M[ 2]*c;

	t = M[ 5];
	M[ 5] = t*c  + M[ 6]*s;
	M[ 6] = t*-s + M[ 6]*c;

	t = M[ 9];
	M[ 9] = t*c  + M[10]*s;
	M[10] = t*-s + M[10]*c;
#endif
}
static void Math_tlApplyYRotation(float *M, float y)
{
	float c, s, t;

	c = tlCos(y);
	s = tlSin(y);

#if !TILE_REVERSE_ROTATION_ENABLED
	t = M[ 0];
	M[ 0] = t*c  + M[ 8]*s;
	M[ 8] = t*-s + M[ 8]*c;

	t = M[ 1];
	M[ 1] = t*c  + M[ 9]*s;
	M[ 9] = t*-s + M[ 9]*c;

	t = M[ 2];
	M[ 2] = t*c  + M[10]*s;
	M[10] = t*-s + M[10]*c;
#else
	t = M[ 0];
	M[ 0] = t*c  + M[ 2]*s;
	M[ 2] = t*-s + M[ 2]*c;

	t = M[ 4];
	M[ 4] = t*c  + M[ 6]*s;
	M[ 6] = t*-s + M[ 6]*c;

	t = M[ 8];
	M[ 8] = t*c  + M[10]*s;
	M[10] = t*-s + M[10]*c;
#endif
}
static void Math_tlApplyZRotation(float *M, float z)
{
	float c, s, t;

	c = tlCos(z);
	s = tlSin(z);

#if !TILE_REVERSE_ROTATION_ENABLED
	t = M[ 0];
	M[ 0] = t*c  + M[ 4]*s;
	M[ 4] = t*-s + M[ 4]*c;

	t = M[ 1];
	M[ 1] = t*c  + M[ 5]*s;
	M[ 5] = t*-s + M[ 5]*c;

	t = M[ 2];
	M[ 2] = t*c  + M[ 6]*s;
	M[ 6] = t*-s + M[ 6]*c;
#else
	t = M[ 0];
	M[ 0] = t*c  + M[ 1]*s;
	M[ 1] = t*-s + M[ 1]*c;

	t = M[ 4];
	M[ 4] = t*c  + M[ 5]*s;
	M[ 5] = t*-s + M[ 5]*c;

	t = M[ 8];
	M[ 8] = t*c  + M[ 9]*s;
	M[ 9] = t*-s + M[ 9]*c;
#endif
}

static void Math_ApplyRotation(float *M, float x, float y, float z)
{
	Math_tlApplyZRotation(M, z);
	Math_tlApplyXRotation(M, x);
	Math_tlApplyYRotation(M, y);
}

static void Math_LoadAffineInverse(float *M, const float *P)
{
	float x, y, z;

	x = -(P[12]*P[ 0] + P[13]*P[ 1] + P[14]*P[ 2]);
	y = -(P[12]*P[ 4] + P[13]*P[ 5] + P[14]*P[ 6]);
	z = -(P[12]*P[ 8] + P[13]*P[ 9] + P[14]*P[10]);

	M[ 0]=P[ 0]; M[ 4]=P[ 1]; M[ 8]=P[ 2]; M[12]=x;
	M[ 1]=P[ 4]; M[ 5]=P[ 5]; M[ 9]=P[ 6]; M[13]=y;
	M[ 2]=P[ 8]; M[ 6]=P[ 9]; M[10]=P[10]; M[14]=z;
	M[ 3]=   0 ; M[ 7]=   0 ; M[11]=   0 ; M[15]=1;
}
static void Math_AffineMultiply(float *M, const float *P, const float *Q)
{
	M[ 0]=P[ 0]*Q[ 0] + P[ 4]*Q[ 1] + P[ 8]*Q[ 2];
	M[ 4]=P[ 0]*Q[ 4] + P[ 4]*Q[ 5] + P[ 8]*Q[ 6];
	M[ 8]=P[ 0]*Q[ 8] + P[ 4]*Q[ 9] + P[ 8]*Q[10];
	M[12]=P[ 0]*Q[12] + P[ 4]*Q[13] + P[ 8]*Q[14] + P[12];

	M[ 1]=P[ 1]*Q[ 0] + P[ 5]*Q[ 1] + P[ 9]*Q[ 2];
	M[ 5]=P[ 1]*Q[ 4] + P[ 5]*Q[ 5] + P[ 9]*Q[ 6];
	M[ 9]=P[ 1]*Q[ 8] + P[ 5]*Q[ 9] + P[ 9]*Q[10];
	M[13]=P[ 1]*Q[12] + P[ 5]*Q[13] + P[ 9]*Q[14] + P[13];

	M[ 2]=P[ 2]*Q[ 0] + P[ 6]*Q[ 1] + P[10]*Q[ 2];
	M[ 6]=P[ 2]*Q[ 4] + P[ 6]*Q[ 5] + P[10]*Q[ 6];
	M[10]=P[ 2]*Q[ 8] + P[ 6]*Q[ 9] + P[10]*Q[10];
	M[14]=P[ 2]*Q[12] + P[ 6]*Q[13] + P[10]*Q[14] + P[14];

	M[ 3]=0;
	M[ 7]=0;
	M[11]=0;
	M[15]=1;
}
static void Math_Multiply3(float *M, const float *P, const float *Q)
{
	M[ 0]=P[ 0]*Q[ 0] + P[ 4]*Q[ 1] + P[ 8]*Q[ 2];
	M[ 4]=P[ 0]*Q[ 4] + P[ 4]*Q[ 5] + P[ 8]*Q[ 6];
	M[ 8]=P[ 0]*Q[ 8] + P[ 4]*Q[ 9] + P[ 8]*Q[10];
	M[12]=0;

	M[ 1]=P[ 1]*Q[ 0] + P[ 5]*Q[ 1] + P[ 9]*Q[ 2];
	M[ 5]=P[ 1]*Q[ 4] + P[ 5]*Q[ 5] + P[ 9]*Q[ 6];
	M[ 9]=P[ 1]*Q[ 8] + P[ 5]*Q[ 9] + P[ 9]*Q[10];
	M[13]=0;

	M[ 2]=P[ 2]*Q[ 0] + P[ 6]*Q[ 1] + P[10]*Q[ 2];
	M[ 6]=P[ 2]*Q[ 4] + P[ 6]*Q[ 5] + P[10]*Q[ 6];
	M[10]=P[ 2]*Q[ 8] + P[ 6]*Q[ 9] + P[10]*Q[10];
	M[14]=0;

	M[ 3]=0;
	M[ 7]=0;
	M[11]=0;
	M[15]=1;
}
static void Math_Multiply4(float *M, const float *P, const float *Q)
{
	M[ 0]=P[ 0]*Q[ 0] + P[ 4]*Q[ 1] + P[ 8]*Q[ 2] + P[12]*Q[ 3];
	M[ 4]=P[ 0]*Q[ 4] + P[ 4]*Q[ 5] + P[ 8]*Q[ 6] + P[12]*Q[ 7];
	M[ 8]=P[ 0]*Q[ 8] + P[ 4]*Q[ 9] + P[ 8]*Q[10] + P[12]*Q[11];
	M[12]=P[ 0]*Q[12] + P[ 4]*Q[13] + P[ 8]*Q[14] + P[12]*Q[15];

	M[ 1]=P[ 1]*Q[ 0] + P[ 5]*Q[ 1] + P[ 9]*Q[ 2] + P[13]*Q[ 3];
	M[ 5]=P[ 1]*Q[ 4] + P[ 5]*Q[ 5] + P[ 9]*Q[ 6] + P[13]*Q[ 7];
	M[ 9]=P[ 1]*Q[ 8] + P[ 5]*Q[ 9] + P[ 9]*Q[10] + P[13]*Q[11];
	M[13]=P[ 1]*Q[12] + P[ 5]*Q[13] + P[ 9]*Q[14] + P[13]*Q[15];

	M[ 2]=P[ 2]*Q[ 0] + P[ 6]*Q[ 1] + P[10]*Q[ 2] + P[14]*Q[ 3];
	M[ 6]=P[ 2]*Q[ 4] + P[ 6]*Q[ 5] + P[10]*Q[ 6] + P[14]*Q[ 7];
	M[10]=P[ 2]*Q[ 8] + P[ 6]*Q[ 9] + P[10]*Q[10] + P[14]*Q[11];
	M[14]=P[ 2]*Q[12] + P[ 6]*Q[13] + P[10]*Q[14] + P[14]*Q[15];

	M[ 3]=P[ 3]*Q[ 0] + P[ 7]*Q[ 1] + P[11]*Q[ 2] + P[15]*Q[ 3];
	M[ 7]=P[ 3]*Q[ 4] + P[ 7]*Q[ 5] + P[11]*Q[ 6] + P[15]*Q[ 7];
	M[11]=P[ 3]*Q[ 8] + P[ 7]*Q[ 9] + P[11]*Q[10] + P[15]*Q[11];
	M[15]=P[ 3]*Q[12] + P[ 7]*Q[13] + P[11]*Q[14] + P[15]*Q[15];
}

static void Math_PointLocalToGlobal( float *OutV, const float *M, const float *P )
{
	OutV[0] = M[ 0]*P[0] + M[ 4]*P[1] + M[ 8]*P[2] + M[12];
	OutV[1] = M[ 1]*P[0] + M[ 5]*P[1] + M[ 9]*P[2] + M[13];
	OutV[2] = M[ 2]*P[0] + M[ 6]*P[1] + M[10]*P[2] + M[14];
}
static void Math_PointGlobalToLocal( float *OutV, const float *M, const float *P )
{
	float Q[3];

	Q[0] = P[0] - M[12];
	Q[1] = P[1] - M[13];
	Q[2] = P[2] - M[14];

	OutV[0] = M[ 0]*Q[0] + M[ 1]*Q[1] + M[ 2]*Q[2];
	OutV[1] = M[ 4]*Q[0] + M[ 5]*Q[1] + M[ 6]*Q[2];
	OutV[2] = M[ 8]*Q[0] + M[ 9]*Q[1] + M[10]*Q[2];
}

static void Math_VectorLocalToGlobal( float *OutV, const float *M, const float *P )
{
	OutV[0] = M[ 0]*P[0] + M[ 4]*P[1] + M[ 8]*P[2];
	OutV[1] = M[ 1]*P[0] + M[ 5]*P[1] + M[ 9]*P[2];
	OutV[2] = M[ 2]*P[0] + M[ 6]*P[1] + M[10]*P[2];
}
static void Math_VectorGlobalToLocal( float *OutV, const float *M, const float *P )
{
	OutV[0] = M[ 0]*P[0] + M[ 1]*P[1] + M[ 2]*P[2];
	OutV[1] = M[ 4]*P[0] + M[ 5]*P[1] + M[ 6]*P[2];
	OutV[2] = M[ 8]*P[0] + M[ 9]*P[1] + M[10]*P[2];
}

static float Math_Dot( const float *P, const float *Q )
{
	return P[0]*Q[0] + P[1]*Q[1] + P[2]*Q[2];
}
static void Math_Cross( float *OutV, const float *P, const float *Q )
{
	OutV[0] = P[1]*Q[2] - P[2]*Q[1];
	OutV[1] = P[2]*Q[0] - P[0]*Q[2];
	OutV[2] = P[0]*Q[1] - P[1]*Q[0];
}

static float Math_Determinant3( const float *M )
{
	float P[3];
	Math_Cross( &P[0], &M[4], &M[8] );
	return Math_Dot( &M[0], &P[0] );
}
static float Math_Determinant4( const float *M )
{
#define xx M[ 0]
#define xy M[ 1]
#define xz M[ 2]
#define xw M[ 3]
#define yx M[ 4]
#define yy M[ 5]
#define yz M[ 6]
#define yw M[ 7]
#define zx M[ 8]
#define zy M[ 9]
#define zz M[10]
#define zw M[11]
#define wx M[12]
#define wy M[13]
#define wz M[14]
#define ww M[15]
	return
		xw*yz*zy*wx - xz*yw*zy*wx - xw*yy*zz*wx + xy*yw*zz*wx +
		xz*yy*zw*wx - xy*yz*zw*wx - xw*yz*zx*wy + xz*yw*zx*wy +
		xw*yx*zz*wy - xx*yw*zz*wy - xz*yx*zw*wy + xx*yz*zw*wy +
		xw*yy*zx*wz - xy*yw*zx*wz - xw*yx*zy*wz + xx*yw*zy*wz +
		xy*yx*zw*wz - xx*yy*zw*wz - xz*yy*zx*ww + xy*yz*zx*ww +
		xz*yx*zy*ww - xx*yz*zy*ww - xy*yx*zz*ww + xx*yy*zz*ww;
#undef xx
#undef xy
#undef xz
#undef xw
#undef yx
#undef yy
#undef yz
#undef yw
#undef zx
#undef zy
#undef zz
#undef zw
#undef wx
#undef wy
#undef wz
#undef ww
}

/*
 * ==========================================================================
 *
 *	MATH
 *
 * ==========================================================================
 */
float tlDegrees(float x) { return x/3.1415926535f*180.0f; }
float tlRadians(float x) { return x/180.0f*3.1415926535f; }

float tlCos(float x) { return cosf(tlRadians(x)); }
float tlSin(float x) { return sinf(tlRadians(x)); }
float tlTan(float x) { return tanf(tlRadians(x)); }

float tlAcos(float x) { return tlDegrees(acosf(x)); }
float tlAsin(float x) { return tlDegrees(asinf(x)); }
float tlAtan(float x) { return tlDegrees(atanf(x)); }
float tlAtan2(float y, float x) { return tlDegrees(atan2f(y, x)); }

float tlSqrt(float x) { return sqrtf(x); }
float tlInvSqrt(float x) { return 1.0f/sqrtf(x); }

float tlClamp(float x, float a, float b) { return x < a ? a : x > b ? b : x; }
float tlSaturate(float x) { return x < 0 ? 0 : x > 1 ? 1 : x; }

float tlLerp(float a, float b, float t) { return a + ( b - a )*t; }

TlVec3 *tlVec3( float x, float y, float z )
{
	static TlVec3 vecs[ 64 ];
	static unsigned int i = 0;

	TlVec3 *pVec;

	pVec = &vecs[ ( i++ )%( sizeof( vecs )/sizeof( vecs[0] ) ) ];

	pVec->x = x;
	pVec->y = y;
	pVec->z = z;

	return pVec;
}
TlVec3 *tlTempVec3( void )
{
	return tlVec3( 0.0f, 0.0f, 0.0f );
}
TlVec3 *tlAddVec3( const TlVec3 *pInP, const TlVec3 *pInQ )
{
	return tlVec3( pInP->x + pInQ->x, pInP->y + pInQ->y, pInP->z + pInQ->z );
}
TlVec3 *tlSubVec3( const TlVec3 *pInP, const TlVec3 *pInQ )
{
	return tlVec3( pInP->x - pInQ->x, pInP->y - pInQ->y, pInP->z - pInQ->z );
}
TlVec3 *tlMulVec3( const TlVec3 *pInP, const TlVec3 *pInQ )
{
	return tlVec3( pInP->x*pInQ->x, pInP->y*pInQ->y, pInP->z*pInQ->z );
}
TlVec3 *tlScaleVec3( const TlVec3 *pInP, float fScale )
{
	return tlVec3( pInP->x*fScale, pInP->y*fScale, pInP->z*fScale );
}
TlVec3 *tlColumn( const TlMat4 *pInM, unsigned int columnAxis )
{
	const float *M;

	M = ( ( const float * )pInM ) + columnAxis*4;
	return tlVec3( M[ 0 ], M[ 4 ], M[ 8 ] );
}
TlVec3 *tlColumnX( const TlMat4 *pInM )
{
	return tlVec3( pInM->xx, pInM->xy, pInM->xz );
}
TlVec3 *tlColumnY( const TlMat4 *pInM )
{
	return tlVec3( pInM->yx, pInM->yy, pInM->yz );
}
TlVec3 *tlColumnZ( const TlMat4 *pInM )
{
	return tlVec3( pInM->zx, pInM->zy, pInM->zz );
}

TlMat4 *tlLoadIdentity( TlMat4 *pOutM )
{
	Math_LoadIdentity( ( float * )pOutM );
	return pOutM;
}
TlMat4 *tlLoadPerspective( TlMat4 *pOutM, float fFovDeg, float fAspect, float fZNear, float fZFar )
{
	Math_LoadPerspective( ( float * )pOutM, fFovDeg, fAspect, fZNear, fZFar );
	return pOutM;
}
TlMat4 *tlLoadOrtho( TlMat4 *pOutM, float fLeft, float fRight, float fBottom, float fTop, float fZNear, float fZFar )
{
	Math_LoadOrtho( ( float * )pOutM, fLeft, fRight, fBottom, fTop, fZNear, fZFar );
	return pOutM;
}
TlMat4 *tlLoadTranslation( TlMat4 *pOutM, float x, float y, float z )
{
	Math_LoadTranslation( ( float * )pOutM, x, y, z );
	return pOutM;
}
TlMat4 *tlLoadTranslationVec( TlMat4 *pOutM, const TlVec3 *pInPosition )
{
	Math_LoadTranslation( ( float * )pOutM, pInPosition->x, pInPosition->y, pInPosition->z );
	return pOutM;
}
TlMat4 *tlLoadRotation( TlMat4 *pOutM, float x, float y, float z )
{
	Math_LoadRotation( ( float * )pOutM, x, y, z );
	return pOutM;
}
TlMat4 *tlLoadRotationVec( TlMat4 *pOutM, const TlVec3 *pInAngles )
{
	Math_LoadRotation( ( float * )pOutM, pInAngles->x, pInAngles->y, pInAngles->z );
	return pOutM;
}
TlMat4 *tlLoadScaling( TlMat4 *pOutM, float x, float y, float z )
{
	Math_LoadScaling( ( float * )pOutM, x, y, z );
	return pOutM;
}
TlMat4 *tlLoadScalingVec( TlMat4 *pOutM, const TlVec3 *pInScaling )
{
	Math_LoadScaling( ( float * )pOutM, pInScaling->x, pInScaling->y, pInScaling->z );
	return pOutM;
}

TlMat4 *tlApplyTranslation( TlMat4 *pInOutM, float x, float y, float z )
{
	Math_ApplyTranslation( ( float * )pInOutM, x, y, z );
	return pInOutM;
}
TlMat4 *tlApplyTranslationVec( TlMat4 *pInOutM, const TlVec3 *pInAxes )
{
	Math_ApplyTranslation( ( float * )pInOutM, pInAxes->x, pInAxes->y, pInAxes->z );
	return pInOutM;
}
TlMat4 *tlApplyXRotation( TlMat4 *pInOutM, float x )
{
	Math_tlApplyXRotation( ( float * )pInOutM, x );
	return pInOutM;
}
TlMat4 *tlApplyYRotation( TlMat4 *pInOutM, float y )
{
	Math_tlApplyYRotation( ( float * )pInOutM, y );
	return pInOutM;
}
TlMat4 *tlApplyZRotation( TlMat4 *pInOutM, float z )
{
	Math_tlApplyZRotation( ( float * )pInOutM, z );
	return pInOutM;
}
TlMat4 *tlApplyRotation( TlMat4 *pInOutM, float x, float y, float z )
{
	Math_ApplyRotation( ( float * )pInOutM, x, y, z );
	return pInOutM;
}
TlMat4 *tlApplyRotationVec( TlMat4 *pInOutM, const TlVec3 *pInAngles )
{
	Math_ApplyRotation( ( float * )pInOutM, pInAngles->x, pInAngles->y, pInAngles->z );
	return pInOutM;
}

TlMat4 *tlLoadAffineInverse( TlMat4 *pOutM, const TlMat4 *pInM )
{
	Math_LoadAffineInverse( ( float * )pOutM, ( const float * )pInM );
	return pOutM;
}
TlMat4 *tlAffineMultiply( TlMat4 *pOutM, const TlMat4 *pInP, const TlMat4 *pInQ )
{
	Math_AffineMultiply( ( float * )pOutM, ( const float * )pInP, ( const float * )pInQ );
	return pOutM;
}
TlMat4 *tlMultiply3( TlMat4 *pOutM, const TlMat4 *pInP, const TlMat4 *pInQ )
{
	Math_Multiply3( ( float * )pOutM, ( const float * )pInP, ( const float * )pInQ );
	return pOutM;
}
TlMat4 *tlMultiply4( TlMat4 *pOutM, const TlMat4 *pInP, const TlMat4 *pInQ )
{
	Math_Multiply4( ( float * )pOutM, ( const float * )pInP, ( const float * )pInQ );
	return pOutM;
}

TlVec3 *tlPointLocalToGlobal( TlVec3 *pOutV, const TlMat4 *pInObjectXf, const TlVec3 *pInPoint )
{
	Math_PointLocalToGlobal( ( float * )pOutV, ( const float * )pInObjectXf, ( const float * )pInPoint );
	return pOutV;
}
TlVec3 *tlPointGlobalToLocal( TlVec3 *pOutV, const TlMat4 *pInObjectXf, const TlVec3 *pInPoint )
{
	Math_PointGlobalToLocal( ( float * )pOutV, ( const float * )pInObjectXf, ( const float * )pInPoint );
	return pOutV;
}

TlVec3 *tlVectorLocalToGlobal( TlVec3 *pOutV, const TlMat4 *pInObjectXf, const TlVec3 *pInVector )
{
	Math_VectorLocalToGlobal( ( float * )pOutV, ( const float * )pInObjectXf, ( const float * )pInVector );
	return pOutV;
}
TlVec3 *tlVectorGlobalToLocal( TlVec3 *pOutV, const TlMat4 *pInObjectXf, const TlVec3 *pInVector )
{
	Math_VectorGlobalToLocal( ( float * )pOutV, ( const float * )pInObjectXf, ( const float * )pInVector );
	return pOutV;
}

float tlDot( const TlVec3 *pInP, const TlVec3 *pInQ )
{
	return Math_Dot( ( const float * )pInP, ( const float * )pInQ );
}
TlVec3 *tlCross( TlVec3 *pOutV, const TlVec3 *pInP, const TlVec3 *pInQ )
{
	Math_Cross( ( float * )pOutV, ( const float * )pInP, ( const float * )pInQ );
	return pOutV;
}

float tlDeterminant3( const TlMat4 *pInM )
{
	return Math_Determinant3( ( const float * )pInM );
}
float tlDeterminant4( const TlMat4 *pInM )
{
	return Math_Determinant4( ( const float * )pInM );
}

void tlScreenToProj( float *x, float *y, float w, float h )
{
	*x = -1.0f + ( *x*2.0f )/w;
	*y =  1.0f - ( *y*2.0f )/h;
}
