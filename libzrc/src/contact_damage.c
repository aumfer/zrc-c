#include <zrc.h>
#include <stdio.h>

static void contact_damage_update_arbiter(cpBody *body, cpArbiter *arbiter, void *data);

void contact_damage_startup(zrc_t *zrc) {
	//printf("contact_damage %zu\n", sizeof(zrc->contact_damage));
}
void contact_damage_shutdown(zrc_t *zrc) {

}
void contact_damage_create(zrc_t *zrc, id_t id, contact_damage_t *contact_damage) {

}
void contact_damage_delete(zrc_t *zrc, id_t id, contact_damage_t *contact_damage) {

}
void contact_damage_update(zrc_t *zrc, id_t id, contact_damage_t *contact_damage) {
	const physics_t *physics = ZRC_GET(zrc, physics, id);
	zrc_assert(physics);
	cpBodyEachArbiter(physics->body, contact_damage_update_arbiter, contact_damage);
}

static void contact_damage_update_arbiter(cpBody *body, cpArbiter *arbiter, void *data) {
	cpSpace *space = cpBodyGetSpace(body);
	zrc_t *zrc = (zrc_t *)cpSpaceGetUserData(space);
	id_t id = (id_t)cpBodyGetUserData(body);
	contact_damage_t *contact_damage = data;

	contact_damage_flags_t flags = contact_damage->flags;
	if ((flags & (CONTACT_DAMAGE_HAS_HIT | CONTACT_DAMAGE_ONE_HIT)) == (CONTACT_DAMAGE_HAS_HIT | CONTACT_DAMAGE_ONE_HIT)) {
		return;
	}

	cpShape *s1, *s2;
	cpArbiterGetShapes(arbiter, &s1, &s2);

	id_t id1 = (id_t)cpShapeGetUserData(s1);
	id_t id2 = (id_t)cpShapeGetUserData(s2);
	id_t hit_id = id == id1 ? id2 : id1;

	const team_t *team = ZRC_GET(zrc, team, id);
	const team_t *hit_team = ZRC_GET(zrc, team, hit_id);
	if (team && hit_team && *team == *hit_team) {
		return;
	}

	if (ZRC_HAS(zrc, contact_damage, hit_id)) {
		// if we hit another contact damage
		const physics_t *physics = ZRC_GET(zrc, physics, id);
		const physics_t *hit_physics = ZRC_GET(zrc, physics, hit_id);
		if (physics && hit_physics) {
			float speed = cpvlengthsq(physics->velocity);
			float hit_speed = cpvlengthsq(hit_physics->velocity);
			if (speed < hit_speed) {
				// fastest wins
				return;
			}
		}
	}

	ZRC_SEND(zrc, damage, hit_id, &contact_damage->damage);

	if ((flags & CONTACT_DAMAGE_HAS_HIT) == 0 && contact_damage->onhit_id != ID_INVALID) {
		cpVect contact = cpArbiterGetPointA(arbiter, 0);
		contact_damage->visual.position[0] = contact.x;
		contact_damage->visual.position[1] = contact.y;
		ZRC_SPAWN(zrc, visual, contact_damage->onhit_id, &contact_damage->visual);
		ZRC_SPAWN(zrc, ttl, contact_damage->onhit_id, &contact_damage->ttl);
	}

	if ((flags & CONTACT_DAMAGE_DESPAWN_ON_HIT) == CONTACT_DAMAGE_DESPAWN_ON_HIT) {
		ZRC_DESPAWN_ALL(zrc, id);
	}

	contact_damage->flags |= CONTACT_DAMAGE_HAS_HIT;
}