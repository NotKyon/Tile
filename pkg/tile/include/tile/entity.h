#ifndef TILE_ENTITY_H
#define TILE_ENTITY_H

#include "const.h"
#include "math.h"

TILE_EXTRNC_ENTER

struct TlEntity_s;
struct TlSurface_s;
struct TlLight_s;
struct TlView_s;

typedef void(*TlThinkFn_t)(struct TlEntity_s *ent);

/*
 * ------
 * Entity
 * ------
 * Represents an object in the scene.
 *
 * Entities consist of transformation and hierarchy data, as well as several
 * components. There can be zero or more surface components, zero or more light
 * components, or an optional view component.
 *
 * Arranging entities into a hierarchy allows the transformations of the child
 * entities to be affected by the parent entities. That is, each child entity is
 * positioned and oriented relative to its parent entity. This can be nested in
 * multiple levels, so you can have a chain of transformations.
 *
 * Imagine for example wheels on a car. The wheels are attached to the car, but
 * when moving they need to appear to rotate, while the car itself should not.
 * Each wheel can be its own child entity of the car's body, then they can
 * rotate separately from the car, while still being relative to the car's body.
 * This allows you to move or turn the car entity without having to do so for
 * each individual wheel.
 *
 * Likewise, you can attach (spot) light entities to the front of the car to
 * simulate headlights. Then they will automatically follow along with the car.
 * You can also attach a "person" that sits inside the car. That person entity
 * might even have a view component attached. That way, you can automatically
 * have a first person perspective from inside the car. You can even switch
 * perspectives by having multiple cameras attached and simply choosing which
 * one is rendered from. That way you can have one set of code to control the
 * car and not have to worry about which camera should be following the car.
 *
 * Components
 * ----------
 * Surface: Represents the geometric data (i.e., polygons) and other information
 *          used to render the entity.
 * Light:   Represents lighting information. e.g., a point or spot light.
 * View:    Represents a renderable view, relative to that entity. Used by
 *          camera entities.
 */
typedef struct TL_CACHELINE_ALIGNED TlEntity_s {
	/* local transformation */
	TlMat4 l_model;
	/* global transformation */
	TlMat4 g_model;
	/* final model-view-projection transformation */
	TlMat4 MVP;

	/* which fields are out-of-date */
	struct {
		/* if set, g_model needs to be updated */
		TlBool gModel:1;
	} recalc;

	/* if valid, called each frame to process the entity */
	TlThinkFn_t Think;

	/* List of surface components */
	struct TlSurface_s *s_head, *s_tail;
	/* List of light components */
	struct TlLight_s *l_head, *l_tail;
	/* Optional view component */
	struct TlView_s *view;

	/* Hierarchy information. p_head and p_tail point to the parent's head and
	`  tail pointers. */
	struct TlEntity_s *prnt, **p_head, **p_tail;
	struct TlEntity_s *head, *tail;
	struct TlEntity_s *prev, *next;
} TlEntity;

/* Entity */
TlEntity *tlNewEntity(TlEntity *prnt);
TlEntity *tlDeleteEntity(TlEntity *ent);
void tlDeleteAllEntities();

void tlResetEntityTransform(TlEntity *ent);
void tlInvalidateEntityBranch(const TlEntity *ent);
void tlInvalidateEntity(TlEntity *ent);
const TlMat4 *tlGetEntityLocalMatrix(TlEntity *ent);
const TlMat4 *tlGetEntityGlobalMatrix(TlEntity *ent);

void tlProcessEntity(TlEntity *ent);
void tlProcessEntityChildren(const TlEntity *ent);
void tlProcessAllEntities();

void tlSetEntityPosition(TlEntity *ent, float x, float y, float z);
void tlSetEntityPositionVec(TlEntity *ent, const TlVec3 *pos);
void tlSetEntityRotation(TlEntity *ent, float x, float y, float z);
void tlSetEntityRotationVec(TlEntity *ent, const TlVec3 *rot);
void tlMoveEntity(TlEntity *ent, float x, float y, float z);
void tlMoveEntityVec(TlEntity *ent, const TlVec3 *distance);
void tlMoveEntityX(TlEntity *ent, float x);
void tlMoveEntityY(TlEntity *ent, float y);
void tlMoveEntityZ(TlEntity *ent, float z);
void tlTurnEntity(TlEntity *ent, float x, float y, float z);
void tlTurnEntityVec(TlEntity *ent, const TlVec3 *angles);
void tlTurnEntityX(TlEntity *ent, float x);
void tlTurnEntityY(TlEntity *ent, float y);
void tlTurnEntityZ(TlEntity *ent, float z);

TlVec3 *tlGetEntityPosition(const TlEntity *ent);
TlVec3 *tlGetEntityAxisX(const TlEntity *ent);
TlVec3 *tlGetEntityAxisY(const TlEntity *ent);
TlVec3 *tlGetEntityAxisZ(const TlEntity *ent);

void tlEntityLocalToGlobal(TlEntity *ent);

TlBool tlIsEntityChildOf(TlEntity *ent, TlEntity *prnt);
void tlSetEntityParent(TlEntity *ent, TlEntity *prnt);
void tlSetEntityParentGlobal(TlEntity *ent, TlEntity *prnt);
TlEntity *tlGetEntityParent(TlEntity *ent);

TlEntity *tlFirstRootEntity();
TlEntity *tlLastRootEntity();
TlEntity *tlEntityParent(const TlEntity *ent);
TlEntity *tlFirstEntity(const TlEntity *ent);
TlEntity *tlLastEntity(const TlEntity *ent);
TlEntity *tlEntityBefore(const TlEntity *ent);
TlEntity *tlEntityAfter(const TlEntity *ent);

TILE_EXTRNC_LEAVE

#endif

