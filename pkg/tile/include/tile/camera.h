#ifndef TILE_CAMERA_H
#define TILE_CAMERA_H

#include "const.h"
#include "view.h"

TILE_EXTRNC_ENTER

struct TlEntity_s;

/* Create a camera entity. (Delete with tlDeleteEntity().) */
struct TlEntity_s *tlNewCamera();

/* Set the current camera entity. */
void tlSetCameraEntity(struct TlEntity_s *cam);
/* Retrieve the current camera entity. */
struct TlEntity_s *tlGetCameraEntity();

/* Set the position of the current camera entity. */
void tlSetCameraPosition(float x, float y, float z);
void tlSetCameraPositionVec(const TlVec3 *pos);
/* Set the rotation of the current camera entity. */
void tlSetCameraRotation(float x, float y, float z);
void tlSetCameraRotationVec(const TlVec3 *rot);

/* Move the current camera entity relative to its current position on its local axes. */
void tlMoveCamera(float x, float y, float z);
void tlMoveCameraVec(const TlVec3 *axes);
/* Move the current camera entity relative to its current position on its local x-axis. (Side to side.) */
void tlMoveCameraX(float x);
/* Move the current camera entity relative to its current position on its local y-axis. (Up and down.) */
void tlMoveCameraY(float y);
/* Move the current camera entity relative to its current position on its local z-axis. (Forward and backward.) */
void tlMoveCameraZ(float z);

/* Turn the current camera entity relative to its current rotation. */
void tlTurnCamera(float x, float y, float z);
void tlTurnCameraVec(const TlVec3 *axes);
/* Turn the current camera entity relative to its current rotation on its local x-axis (pitch). (up/down.) */
void tlTurnCameraX(float x);
/* Turn the current camera entity relative to its current rotation on its local y-axis (yaw). (left/right.) */
void tlTurnCameraY(float y);
/* Turn the current camera entity relative to its current rotation on its local z-axis (roll). */
void tlTurnCameraZ(float z);

/* Retrieve the current camera's viewport's x-position in pixels. */
int tlGetCameraViewX();
/* Retrieve the current camera's viewport's y-position in pixels. */
int tlGetCameraViewY();
/* Retrieve the current camera's viewport's x-size (width) in pixels. */
int tlGetCameraViewWidth();
/* Retrieve the current camera's viewport's y-size (height) in pixels. */
int tlGetCameraViewHeight();

/* Adjust the camera's projection for perspective rendering. */
void tlSetCameraPerspective(float fov);
/* Adjust the camera's projection for orthographic rendering. */
void tlSetCameraOrtho(float left, float top, float right, float bottom);
/* Adjust the camera's projection's near and far ranges. Neither value should be 0. */
void tlSetCameraRange(float zn, float zf);

/* Retrieve the camera projection's perspective field-of-view value. */
float tlGetCameraPerspectiveFOV();
/* Retrieve the camera projection's orthographic left value. */
float tlGetCameraOrthoLeft();
/* Retrieve the camera projection's orthographic top value. */
float tlGetCameraOrthoTop();
/* Retrieve the camera projection's orthographic right value. */
float tlGetCameraOrthoRight();
/* Retrieve the camera projection's orthographic bottom value. */
float tlGetCameraOrthoBottom();
/* Retrieve the camera projection's near range. */
float tlGetCameraRangeNear();
/* Retrieve the camera projection's far range. */
float tlGetCameraRangeFar();

/* Enable clearing the view's color buffer each time the camera is rendered. */
void tlEnableCameraClearColor();
/* Disable clearing the view's color buffer each time the camera is rendered. */
void tlDisableCameraClearColor();
/* Retrieve whether the view's color buffer will be cleared each time the camera is rendered. */
TlBool tlIsCameraClearColorEnabled();

/* Enable clearing the view's depth buffer each time the camera is rendered. */
void tlEnableCameraClearDepth();
/* Disable clearing the view's depth buffer each time the camera is rendered. */
void tlDisableCameraClearDepth();
/* Retrieve whether the view's depth buffer will be cleared each time the camera is rendered. */
TlBool tlIsCameraClearDepthEnabled();

/* Enable clearing the view's stencil buffer each time the camera is rendered. */
void tlEnableCameraClearStencil();
/* Disable clearing the view's stencil buffer each time the camera is rendered. */
void tlDisableCameraClearStencil();
/* Retrieve whether the view's stencil buffer will be cleared each time the camera is rendered. */
TlBool tlIsCameraClearStencilEnabled();

/* Set the color to use when this camera clears the view's color buffer. */
void tlSetCameraClearColor(float r, float g, float b, float a);
/* Retrieve the red channel of the camera's clearing color. */
float tlGetCameraClearColorR();
/* Retrieve the green channel of the camera's clearing color. */
float tlGetCameraClearColorG();
/* Retrieve the blue channel of the camera's clearing color. */
float tlGetCameraClearColorB();
/* Retrieve the alpha channel of the camera's clearing color. */
float tlGetCameraClearColorA();

/* Set the depth value (0.0 ... 1.0) to use when this camera clears the view's depth buffer. */
void tlSetCameraClearDepth(float z);
/* Retrieve the depth value (0.0 ... 1.0) used when this camera clears the view's depth buffer. */
float tlGetCameraClearDepth();

/* Set the stencil value (0 ... 255) to use when this camera clears the view's stencil buffer. */
void tlSetCameraClearStencil(unsigned char mask);
/* Retrieve the stencil value (0 ... 255) used when this camera clears the view's stencil buffer. */
unsigned char tlGetCameraClearStencil();

/* Set a proportional viewport for the current camera, which is screen-size independent. */
void tlSetCameraAutoVP(float l, float t, float r, float b);
/* Set a fixed viewport for the current camera, which must be at the given pixel locations. */
void tlSetCameraRealVP(int l, int t, int r, int b);
/* Set the aspect ratio the current camera's viewport should stick to. */
void tlSetCameraAutoAspect(double aspectRatio, TlAspect_t aspectMode);

TILE_EXTRNC_LEAVE

#endif

