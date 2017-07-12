#include <tile/entity.h>
#include <tile/math.h>
#include <tile/surface.h>
#include <tile/light.h>
#include <tile/view.h>

/*
 * ==========================================================================
 *
 *	ENTITY
 *
 * ==========================================================================
 */

#ifndef ENTITY_DIRTY_GLOBAL_ENABLED
# define ENTITY_DIRTY_GLOBAL_ENABLED 0
#endif

static TlEntity *g_ent_head = (TlEntity *)0;
static TlEntity *g_ent_tail = (TlEntity *)0;

TlEntity *tlNewEntity(TlEntity *prnt) {
	TlEntity *ent;

	ent = tlAllocStruct(TlEntity);

	tlLoadIdentity(&ent->l_model);
	tlLoadIdentity(&ent->g_model);
	tlLoadIdentity(&ent->MVP);

	ent->recalc.gModel = TRUE;

	ent->Think = (TlThinkFn_t)0;

	ent->s_head = (TlSurface *)0;
	ent->s_tail = (TlSurface *)0;
	ent->l_head = (TlLight *)0;
	ent->l_tail = (TlLight *)0;
	ent->view = (TlView *)0;

	ent->prnt = prnt;
	ent->p_head = prnt ? &prnt->head : &g_ent_head;
	ent->p_tail = prnt ? &prnt->tail : &g_ent_tail;
	ent->head = (TlEntity *)0;
	ent->tail = (TlEntity *)0;
	ent->next = (TlEntity *)0;
	if( ( ent->prev = *ent->p_tail ) != (TlEntity *)0 ) {
		( *ent->p_tail )->next = ent;
	} else {
		*ent->p_head = ent;
	}
	*ent->p_tail = ent;

	return ent;
}
TlEntity *tlDeleteEntity(TlEntity *ent) {
	if( !ent ) {
		return (TlEntity *)0;
	}

	while( ent->head != (TlEntity *)0 ) {
		tlDeleteEntity(ent->head);
	}

	while( ent->s_head != (TlSurface *)0 ) {
		tlDeleteSurface(ent->s_head);
	}

	if( ent->prev != (TlEntity *)0 ) {
		ent->prev->next = ent->next;
	} else {
		*ent->p_head = ent->next;
	}

	if( ent->next != (TlEntity *)0 ) {
		ent->next->prev = ent->prev;
	} else {
		*ent->p_tail = ent->prev;
	}

	return (TlEntity *)tlMemory((void *)ent, 0);
}
void tlDeleteAllEntities() {
	while( g_ent_head != (TlEntity *)0 ) {
		tlDeleteEntity(g_ent_head);
	}
}
void tlResetEntityTransform(TlEntity *ent) {
	tlLoadIdentity(&ent->l_model);
	tlInvalidateEntity(ent);
}
void tlInvalidateEntityBranch(const TlEntity *ent) {
#if ENTITY_DIRTY_GLOBAL_ENABLED
	TlEntity *chld;

	for(chld=ent->head; chld!=(TlEntity *)0; chld=chld->next) {
		chld->recalc.gModel = TRUE;

		tlInvalidateEntityBranch(chld);
	}
#else
	(void)ent;
#endif
}
void tlInvalidateEntity(TlEntity *ent) {
#if ENTITY_DIRTY_GLOBAL_ENABLED
	ent->recalc.gModel = TRUE;
	tlInvalidateEntityBranch(ent);
#else
	(void)ent;
#endif
}
const TlMat4 *tlGetEntityLocalMatrix(TlEntity *ent) {
	return &ent->l_model;
}
const TlMat4 *tlGetEntityGlobalMatrix(TlEntity *ent) {
	TlEntity *prnt;

	prnt = ent->prnt;
	if( !prnt ) {
		return &ent->l_model;
	}

#if ENTITY_DIRTY_GLOBAL_ENABLED
	if( !ent->recalc.gModel ) {
		return &ent->g_model;
	}
#endif

	tlAffineMultiply(&ent->g_model, tlGetEntityGlobalMatrix(prnt), &ent->l_model);
	ent->recalc.gModel = FALSE;

#if ENTITY_DIRTY_GLOBAL_ENABLED
	tlInvalidateEntityBranch(ent);
#endif
	return &ent->g_model;
}
void tlProcessEntity(TlEntity *ent) {
	if( ent->Think != NULL ) {
		ent->Think(ent);
	}
}
void tlProcessEntityChildren(const TlEntity *ent) {
	TlEntity *chld;

	for(chld=ent->head; chld!=(TlEntity *)0; chld=chld->next) {
		tlProcessEntity(chld);
		tlProcessEntityChildren(chld);
	}
}
void tlProcessAllEntities() {
	TlEntity *ent;

	for(ent=g_ent_head; ent!=(TlEntity *)0; ent=ent->next) {
		tlProcessEntity(ent);
		tlProcessEntityChildren(ent);
	}
}
void tlSetEntityPosition(TlEntity *ent, float x, float y, float z) {
	ent->l_model.xw = x;
	ent->l_model.yw = y;
	ent->l_model.zw = z;

	tlInvalidateEntity(ent);
}
void tlSetEntityPositionVec(TlEntity *ent, const TlVec3 *pos) {
	TL_ASSERT( ent != NULL && "Entity cannot be NULL" );
	TL_ASSERT( pos != NULL && "Position vector cannot be NULL" );

	ent->l_model.xw = pos->x;
	ent->l_model.yw = pos->y;
	ent->l_model.zw = pos->z;

	tlInvalidateEntity(ent);
}
void tlSetEntityRotation(TlEntity *ent, float x, float y, float z) {
	float xyz[3];

	xyz[0] = ent->l_model.xw;
	xyz[1] = ent->l_model.yw;
	xyz[2] = ent->l_model.zw;

	tlLoadRotation(&ent->l_model, x, y, z);

	ent->l_model.xw = xyz[0];
	ent->l_model.yw = xyz[1];
	ent->l_model.zw = xyz[2];

	tlInvalidateEntity(ent);
}
void tlSetEntityRotationVec(TlEntity *ent, const TlVec3 *rot) {
	float xyz[3];

	TL_ASSERT( ent != NULL && "Entity cannot be NULL" );
	TL_ASSERT( rot != NULL && "Rotation vector cannot be NULL" );

	xyz[0] = ent->l_model.xw;
	xyz[1] = ent->l_model.yw;
	xyz[2] = ent->l_model.zw;

	tlLoadRotation(&ent->l_model, rot->x, rot->y, rot->z);

	ent->l_model.xw = xyz[0];
	ent->l_model.yw = xyz[1];
	ent->l_model.zw = xyz[2];

	tlInvalidateEntity(ent);
}
void tlMoveEntity(TlEntity *ent, float x, float y, float z) {
	tlApplyTranslation(&ent->l_model, x, y, z);
	tlInvalidateEntity(ent);
}
void tlMoveEntityVec(TlEntity *ent, const TlVec3 *axes) {
	TL_ASSERT( ent != NULL && "Entity cannot be NULL" );
	TL_ASSERT( axes != NULL && "Movement vector cannot be NULL" );
	
	tlApplyTranslation(&ent->l_model, axes->x, axes->y, axes->z);
	tlInvalidateEntity(ent);
}
void tlMoveEntityX(TlEntity *ent, float x) {
	tlMoveEntity(ent, x, 0, 0);
}
void tlMoveEntityY(TlEntity *ent, float y) {
	tlMoveEntity(ent, 0, y, 0);
}
void tlMoveEntityZ(TlEntity *ent, float z) {
	tlMoveEntity(ent, 0, 0, z);
}
void tlTurnEntity(TlEntity *ent, float x, float y, float z) {
	tlApplyRotation(&ent->l_model, x, y, z);
	tlInvalidateEntity(ent);
}
void tlTurnEntityVec(TlEntity *ent, const TlVec3 *axes) {
	TL_ASSERT( ent != NULL && "Entity cannot be NULL" );
	TL_ASSERT( axes != NULL && "Rotation vector cannot be NULL" );
	
	tlApplyRotation(&ent->l_model, axes->x, axes->y, axes->z);
	tlInvalidateEntity(ent);
}
void tlTurnEntityX(TlEntity *ent, float x) {
	tlApplyXRotation(&ent->l_model, x);
	tlInvalidateEntity(ent);
}
void tlTurnEntityY(TlEntity *ent, float y) {
	tlApplyYRotation(&ent->l_model, y);
	tlInvalidateEntity(ent);
}
void tlTurnEntityZ(TlEntity *ent, float z) {
	tlApplyZRotation(&ent->l_model, z);
	tlInvalidateEntity(ent);
}

TlVec3 *tlGetEntityPosition(const TlEntity *ent) {
	return tlVec3(ent->l_model.xw, ent->l_model.yw, ent->l_model.zw);
}
TlVec3 *tlGetEntityAxisX(const TlEntity *ent) {
	return tlColumnX(&ent->l_model);
}
TlVec3 *tlGetEntityAxisY(const TlEntity *ent) {
	return tlColumnY(&ent->l_model);
}
TlVec3 *tlGetEntityAxisZ(const TlEntity *ent) {
	return tlColumnZ(&ent->l_model);
}

void tlEntityLocalToGlobal(TlEntity *ent) {
	TlMat4 ginv, tmp;

	if( !ent->prnt )
		return;

	tlLoadAffineInverse(&ginv, tlGetEntityGlobalMatrix(ent->prnt));
	tlAffineMultiply(&tmp, &ginv, &ent->l_model);
	memcpy(&ent->g_model, &ent->l_model, sizeof(TlMat4));
	memcpy(&ent->l_model, &tmp, sizeof(TlMat4));

	tlInvalidateEntity(ent);
}

TlBool tlIsEntityChildOf(TlEntity *ent, TlEntity *prnt) {
	if( ent->prnt==prnt ) {
		return TRUE;
	}

	if( !prnt ) {
		return TRUE;
	}

	ent = ent->prnt;
	while( ent != (TlEntity *)0 ) {
		ent = ent->prnt;
		if( ent==prnt ) {
			return TRUE;
		}
	}

	return FALSE;
}
void tlSetEntityParent(TlEntity *ent, TlEntity *prnt) {
	if( !prnt ) {
		ent->prnt = (TlEntity *)0;

		if( ent->prev != (TlEntity *)0 ) {
			ent->prev->next = ent->next;
		} else {
			*ent->p_head = ent->next;
		}

		if( ent->next != (TlEntity *)0 ) {
			ent->next->prev = ent->prev;
		} else {
			*ent->p_tail = ent->prev;
		}

		ent->p_head = &g_ent_head;
		ent->p_tail = &g_ent_tail;
	} else {
		if( tlIsEntityChildOf(prnt, ent) ) {
			tlSetEntityParent(prnt, ent->prnt);
		}

		ent->prnt = prnt;

		if( ent->prev != (TlEntity *)0 ) {
			ent->prev->next = ent->next;
		} else {
			*ent->p_head = ent->next;
		}

		if( ent->next != (TlEntity *)0 ) {
			ent->next->prev = ent->prev;
		} else {
			*ent->p_tail = ent->prev;
		}

		ent->p_head = &prnt->head;
		ent->p_tail = &prnt->tail;
	}

	ent->next = (TlEntity *)0;
	if( ( ent->prev = *ent->p_tail ) != (TlEntity *)0 ) {
		( *ent->p_tail )->next = ent;
	} else {
		*ent->p_head = ent;
	}
	*ent->p_tail = ent;

	tlInvalidateEntity(ent);
}
void tlSetEntityParentGlobal(TlEntity *ent, TlEntity *prnt) {
	memcpy(&ent->l_model, tlGetEntityGlobalMatrix(ent), sizeof(TlMat4));

	tlSetEntityParent(ent, prnt);
	tlEntityLocalToGlobal(ent);
}
TlEntity *tlGetEntityParent(TlEntity *ent) {
	return ent->prnt;
}

TlEntity *tlFirstRootEntity() {
	return g_ent_head;
}
TlEntity *tlLastRootEntity() {
	return g_ent_tail;
}
TlEntity *tlEntityParent(const TlEntity *ent) {
	return ent->prnt;
}
TlEntity *tlFirstEntity(const TlEntity *ent) {
	return ent->head;
}
TlEntity *tlLastEntity(const TlEntity *ent) {
	return ent->tail;
}
TlEntity *tlEntityBefore(const TlEntity *ent) {
	return ent->prev;
}
TlEntity *tlEntityAfter(const TlEntity *ent) {
	return ent->next;
}

