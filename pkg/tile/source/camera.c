#include <tile/camera.h>
#include <tile/entity.h>
#include <tile/view.h>

/*
 * ==========================================================================
 *
 *	CAMERA SYSTEM
 *
 * ==========================================================================
 */
static TlEntity *g_camera = (TlEntity *)0;

TlEntity *tlNewCamera() {
	TlEntity *cam;

	cam = tlNewEntity((TlEntity *)0);
	tlNewView(cam);

	if (!g_camera)
		g_camera = cam;

	return cam;
}

void tlSetCameraEntity(TlEntity *cam) {
	if (!cam || !cam->view)
		return;

	g_camera = cam;
}
TlEntity *tlGetCameraEntity() {
	return g_camera;
}

void tlSetCameraPosition(float x, float y, float z) {
	tlSetEntityPosition(g_camera, x, y, z);
}
void tlSetCameraPositionVec(const TlVec3 *pos) {
	tlSetEntityPositionVec(g_camera, pos);
}
void tlSetCameraRotation(float x, float y, float z) {
	tlSetEntityRotation(g_camera, x, y, z);
}
void tlSetCameraRotationVec(const TlVec3 *rot) {
	tlSetEntityRotationVec(g_camera, rot);
}

void tlMoveCamera(float x, float y, float z) {
	tlMoveEntity(g_camera, x, y, z);
}
void tlMoveCameraVec(const TlVec3 *axes) {
	tlMoveEntityVec(g_camera, axes);
}
void tlMoveCameraX(float x) {
	tlMoveEntityX(g_camera, x);
}
void tlMoveCameraY(float y) {
	tlMoveEntityY(g_camera, y);
}
void tlMoveCameraZ(float z) {
	tlMoveEntityZ(g_camera, z);
}

void tlTurnCamera(float x, float y, float z) {
	tlTurnEntity(g_camera, x, y, z);
}
void tlTurnCameraVec(const TlVec3 *axes) {
	tlTurnEntityVec(g_camera, axes);
}
void tlTurnCameraX(float x) {
	tlTurnEntityX(g_camera, x);
}
void tlTurnCameraY(float y) {
	tlTurnEntityY(g_camera, y);
}
void tlTurnCameraZ(float z) {
	tlTurnEntityZ(g_camera, z);
}

int tlGetCameraViewX() {
	return tlGetViewX(g_camera->view);
}
int tlGetCameraViewY() {
	return tlGetViewY(g_camera->view);
}
int tlGetCameraViewWidth() {
	return tlGetViewWidth(g_camera->view);
}
int tlGetCameraViewHeight() {
	return tlGetViewHeight(g_camera->view);
}

void tlSetCameraPerspective(float fov) {
	tlSetViewPerspective(g_camera->view, fov);
}
void tlSetCameraOrtho(float left, float top, float right, float bottom) {
	tlSetViewOrtho(g_camera->view, left, top, right, bottom);
}
void tlSetCameraRange(float zn, float zf) {
	tlSetViewRange(g_camera->view, zn, zf);
}

float tlGetCameraPerspectiveFOV() {
	return tlGetViewPerspectiveFOV(g_camera->view);
}
float tlGetCameraOrthoLeft() {
	return tlGetViewOrthoLeft(g_camera->view);
}
float tlGetCameraOrthoTop() {
	return tlGetViewOrthoTop(g_camera->view);
}
float tlGetCameraOrthoRight() {
	return tlGetViewOrthoRight(g_camera->view);
}
float tlGetCameraOrthoBottom() {
	return tlGetViewOrthoBottom(g_camera->view);
}
float tlGetCameraRangeNear() {
	return tlGetViewRangeNear(g_camera->view);
}
float tlGetCameraRangeFar() {
	return tlGetViewRangeFar(g_camera->view);
}

void tlEnableCameraClearColor() {
	tlEnableViewClearColor(g_camera->view);
}
void tlDisableCameraClearColor() {
	tlDisableViewClearColor(g_camera->view);
}
TlBool tlIsCameraClearColorEnabled() {
	return tlIsViewClearColorEnabled(g_camera->view);
}

void tlEnableCameraClearDepth() {
	tlEnableViewClearDepth(g_camera->view);
}
void tlDisableCameraClearDepth() {
	tlDisableViewClearDepth(g_camera->view);
}
TlBool tlIsCameraClearDepthEnabled() {
	return tlIsViewClearDepthEnabled(g_camera->view);
}

void tlEnableCameraClearStencil() {
	tlEnableViewClearStencil(g_camera->view);
}
void tlDisableCameraClearStencil() {
	tlDisableViewClearStencil(g_camera->view);
}
TlBool tlIsCameraClearStencilEnabled() {
	return tlIsViewClearStencilEnabled(g_camera->view);
}

void tlSetCameraClearColor(float r, float g, float b, float a) {
	tlSetViewClearColor(g_camera->view, r, g, b, a);
}
float tlGetCameraClearColorR() {
	return tlGetViewClearColorR(g_camera->view);
}
float tlGetCameraClearColorG() {
	return tlGetViewClearColorG(g_camera->view);
}
float tlGetCameraClearColorB() {
	return tlGetViewClearColorB(g_camera->view);
}
float tlGetCameraClearColorA() {
	return tlGetViewClearColorA(g_camera->view);
}

void tlSetCameraClearDepth(float z) {
	tlSetViewClearDepth(g_camera->view, z);
}
float tlGetCameraClearDepth() {
	return tlGetViewClearDepth(g_camera->view);
}

void tlSetCameraClearStencil(unsigned char mask) {
	tlSetViewClearStencil(g_camera->view, mask);
}
unsigned char tlGetCameraClearStencil() {
	return tlGetViewClearStencil(g_camera->view);
}

void tlSetCameraAutoVP(float l, float t, float r, float b) {
	tlSetViewAutoVP(g_camera->view, l, t, r, b);
}
void tlSetCameraRealVP(int l, int t, int r, int b) {
	tlSetViewRealVP(g_camera->view, l, t, r, b);
}
void tlSetCameraAutoAspect(double aspectRatio, TlAspect_t aspectMode) {
	tlSetViewAutoAspect(g_camera->view, aspectRatio, aspectMode);
}

