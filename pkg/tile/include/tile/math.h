#ifndef TILE_MATH_H
#define TILE_MATH_H

#include "const.h"

TILE_EXTRNC_ENTER

/* Math */
float tlDegrees(float x);
float tlRadians(float x);

float tlCos(float x);
float tlSin(float x);
float tlTan(float x);

float tlAcos(float x);
float tlAsin(float x);
float tlAtan(float x);
float tlAtan2(float y, float x);

float tlSqrt(float x);
float tlInvSqrt(float x);

float tlClamp(float x, float a, float b);
float tlSaturate(float x);

float tlLerp(float a, float b, float t);

/*
	MATRIX LAYOUT
	-------------

	M[ 0]=xx; M[ 4]=xy; M[ 8]=xz; M[12]=xw;
	M[ 1]=yx; M[ 5]=yy; M[ 9]=yz; M[13]=yw;
	M[ 2]=zx; M[ 6]=zy; M[10]=zz; M[14]=zw;
	M[ 3]=wx; M[ 7]=wy; M[11]=wz; M[15]=ww;

	M[12] = xw = translation x
	M[13] = yw = translation y
	M[14] = zw = translation z
*/

typedef struct TlVec3_s
{
	float x, y, z;
} TlVec3;
typedef struct TlColor_s
{
	float r, g, b, a;
} TlColor;

typedef struct TlMat3_s
{
	float xx, yx, zx;
	float xy, yy, zy;
	float xz, yz, zz;
} TlMat3;

typedef struct TlMat4_s
{
	float xx, yx, zx, wx;
	float xy, yy, zy, wy;
	float xz, yz, zz, wz;
	float xw, yw, zw, ww;
} TlMat4;

TlVec3 *tlVec3( float x, float y, float z );
TlVec3 *tlTempVec3( void );
TlVec3 *tlAddVec3( const TlVec3 *pInP, const TlVec3 *pInQ );
TlVec3 *tlSubVec3( const TlVec3 *pInP, const TlVec3 *pInQ );
TlVec3 *tlMulVec3( const TlVec3 *pInP, const TlVec3 *pInQ );
TlVec3 *tlScaleVec3( const TlVec3 *pInP, float fScale );
TlVec3 *tlColumn( const TlMat4 *pInM, unsigned int columnAxis );
TlVec3 *tlColumnX( const TlMat4 *pInM );
TlVec3 *tlColumnY( const TlMat4 *pInM );
TlVec3 *tlColumnZ( const TlMat4 *pInM );

TlMat4 *tlLoadIdentity( TlMat4 *pOutM );
TlMat4 *tlLoadPerspective( TlMat4 *pOutM, float fFovDeg, float fAspect, float fZNear, float fZFar );
TlMat4 *tlLoadOrtho( TlMat4 *pOutM, float fLeft, float fRight, float fBottom, float fTop, float fZNear, float fZFar );
TlMat4 *tlLoadTranslation( TlMat4 *pOutM, float x, float y, float z );
TlMat4 *tlLoadTranslationVec( TlMat4 *pOutM, const TlVec3 *pInPosition );
TlMat4 *tlLoadRotation( TlMat4 *pOutM, float x, float y, float z );
TlMat4 *tlLoadRotationVec( TlMat4 *pOutM, const TlVec3 *pInAngles );
TlMat4 *tlLoadScaling( TlMat4 *pOutM, float x, float y, float z );
TlMat4 *tlLoadScalingVec( TlMat4 *pOutM, const TlVec3 *pInScaling );

TlMat4 *tlApplyTranslation( TlMat4 *pInOutM, float x, float y, float z );
TlMat4 *tlApplyTranslationVec( TlMat4 *pInOutM, const TlVec3 *pInAxes );

TlMat4 *tlApplyXRotation( TlMat4 *pInOutM, float x );
TlMat4 *tlApplyYRotation( TlMat4 *pInOutM, float y );
TlMat4 *tlApplyZRotation( TlMat4 *pInOutM, float z );
TlMat4 *tlApplyRotation( TlMat4 *pInOutM, float x, float y, float z );
TlMat4 *tlApplyRotationVec( TlMat4 *pInOutM, const TlVec3 *pInAngles );

TlMat4 *tlLoadAffineInverse( TlMat4 *pOutM, const TlMat4 *pInM );
TlMat4 *tlAffineMultiply( TlMat4 *pOutM, const TlMat4 *pInP, const TlMat4 *pInQ );
TlMat4 *tlMultiply3( TlMat4 *pOutM, const TlMat4 *pInP, const TlMat4 *pInQ );
TlMat4 *tlMultiply4( TlMat4 *pOutM, const TlMat4 *pInP, const TlMat4 *pInQ );

TlVec3 *tlPointLocalToGlobal( TlVec3 *pOutV, const TlMat4 *pInObjectXf, const TlVec3 *pInPoint );
TlVec3 *tlPointGlobalToLocal( TlVec3 *pOutV, const TlMat4 *pInObjectXf, const TlVec3 *pInPoint );
TlVec3 *tlVectorLocalToGlobal( TlVec3 *pOutV, const TlMat4 *pInObjectXf, const TlVec3 *pInVector );
TlVec3 *tlVectorGlobalToLocal( TlVec3 *pOutV, const TlMat4 *pInObjectXf, const TlVec3 *pInVector );

float tlDot( const TlVec3 *pInP, const TlVec3 *pInQ );
TlVec3 *tlCross( TlVec3 *pOutV, const TlVec3 *pInP, const TlVec3 *pInQ );

float tlDeterminant3( const TlMat4 *pInM );
float tlDeterminant4( const TlMat4 *pInM );

void tlScreenToProj( float *x, float *y, float w, float h );

TILE_EXTRNC_LEAVE

#endif

