#include <zrc_host.h>
#include <stdio.h>

static void cast_tur_proj_attack(zrc_t *zrc, const ability_t *ability, id_t caster_id, const ability_target_t *target) {
	zrc_host_t *zrc_host = zrc->user;

	physics_t *physics = ZRC_GET(zrc, physics, caster_id);

	id_t proj_id = zrc_host_put(zrc_host, guid_create());
	float proj_speed = 250;

	cpVect front = cpvforangle(physics->angle);
	cpVect target_point = cpv(target->point[0], target->point[1]);
	cpVect dir = cpvnormalize(cpvsub(target_point, physics->position));
	physics_t proj_physics = {
		.collide_flags = ~0,
		.collide_mask = ~0,
		.response_mask = ~0,
		.radius = 0.1f,
		.position = cpvadd(physics->position, cpvmult(front, physics->radius)),
		.velocity = cpvmult(dir, proj_speed)
	};
	ZRC_SPAWN(zrc, physics, proj_id, &proj_physics);
	ZRC_SPAWN(zrc, ttl, proj_id, &(ttl_t) {
		.ttl = 2
	});
	ZRC_SPAWN(zrc, visual, proj_id, &(visual_t) {
		.color = color_random(255)
	});
	contact_damage_t contact_damage = {
		.damage = {
			.from = caster_id,
			.health = 10
		},
		.flags = CONTACT_DAMAGE_DESPAWN_ON_HIT,
		.onhit_id = zrc_host_put(zrc_host, guid_create()),
		.visual = {
			.color = color_random(255),
			.size = { 10, 10 }
		},
		.ttl = {
			.ttl = 0.5f
		}
	};
	ZRC_SPAWN(zrc, contact_damage, proj_id, &contact_damage);
}
static void cast_blink(zrc_t *zrc, const ability_t *ability, id_t caster_id, const ability_target_t *target) {
	physics_t *physics = ZRC_GET_WRITE(zrc, physics, caster_id);
	physics->position.x = target->point[0];
	physics->position.y = target->point[1];
}
static void cast_fix_proj_attack(zrc_t *zrc, const ability_t *ability, id_t caster_id, const ability_target_t *target) {
	zrc_host_t *zrc_host = zrc->user;

	physics_t *physics = ZRC_GET(zrc, physics, caster_id);

	id_t proj_id = zrc_host_put(zrc_host, guid_create());
	float proj_speed = 100;

	cpVect front = cpvforangle(physics->angle);
	physics_t proj_physics = {
		.collide_flags = ~0,
		.collide_mask = ~0,
		.response_mask = ~0,
		.radius = 0.25f,
		.position = cpvadd(physics->position, cpvmult(front, physics->radius)),
		.velocity = cpvmult(front, proj_speed)
	};
	ZRC_SPAWN(zrc, physics, proj_id, &proj_physics);
	ZRC_SPAWN(zrc, ttl, proj_id, &(ttl_t) {
		.ttl = 3
	});
	ZRC_SPAWN(zrc, visual, proj_id, &(visual_t) {
		.color = color_random(255)
	});
	contact_damage_t contact_damage = {
		.damage = {
			.from = caster_id,
			.health = 20
		},
		.flags = CONTACT_DAMAGE_DESPAWN_ON_HIT,
		.onhit_id = zrc_host_put(zrc_host, guid_create()),
		.visual = {
			.color = color_random(255),
			.size = { 10, 10 }
		},
		.ttl = {
			.ttl = 0.5f
		}
	};
	ZRC_SPAWN(zrc, contact_damage, proj_id, &contact_damage);
}
static void cast_target_nuke(zrc_t *zrc, const ability_t *ability, id_t caster_id, const ability_target_t *target) {
	zrc_host_t *zrc_host = zrc->user;

	damage_t damage = {
		.from = caster_id,
		.health = 30
	};
	ZRC_SEND(zrc, damage, target->unit, &damage);

	physics_t *target_physics = ZRC_GET(zrc, physics, target->unit);
	if (target_physics) {
		id_t hit_id = zrc_host_put(zrc_host, guid_create());
		ZRC_SPAWN(zrc, ttl, hit_id, &(ttl_t) {
			.ttl = 0.5f
		});
		ZRC_SPAWN(zrc, visual, hit_id, &(visual_t) {
			.color = color_random(255)
		});
		visual_t visual = {
			.color = color_random(255),
			.size = { 10, 10 },
			.position = { target_physics->position.x, target_physics->position.y }
		};
		ZRC_SPAWN(zrc, visual, hit_id, &visual);
	}
}

void zrc_host_startup(zrc_host_t *zrc_host, zrc_t *zrc) {
	zrc->user = zrc_host;

	kh_resize(zhash, &zrc_host->entities, MAX_ENTITIES);

	zrc->ability[ABILITY_TUR_PROJ_ATTACK].cast = cast_tur_proj_attack;
	zrc->ability[ABILITY_BLINK].cast = cast_blink;
	zrc->ability[ABILITY_FIX_PROJ_ATTACK].cast = cast_fix_proj_attack;
	zrc->ability[ABILITY_TARGET_NUKE].cast = cast_target_nuke;
}
void zrc_host_shutdown(zrc_host_t *zrc_host) {
}

void zrc_host_tick(zrc_host_t *zrc_host, zrc_t *zrc) {
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		registry_t *read = ZRC_GET_READ(zrc, registry, i);
		registry_t *prev = ZRC_GET_PREV(zrc, registry, i);
		if (*read == 0 && *prev != 0) {
			//printf("deleting %d\n", i);
			kh_del(zhash, &zrc_host->entities, i);
		}
	}
}

id_t zrc_host_put(zrc_host_t *zrc_host, guid_t guid) {
	int absent;
	khint_t k = kh_put(zhash, &zrc_host->entities, guid, &absent);
	return (id_t)k;
}
id_t zrc_host_del(zrc_host_t *zrc_host, guid_t guid) {
	khint_t k = kh_get(zhash, &zrc_host->entities, guid);
	kh_del(zhash, &zrc_host->entities, k);
	return (id_t)k;
}
id_t zrc_host_get(zrc_host_t *zrc_host, guid_t guid) {
	khint_t k = kh_get(zhash, &zrc_host->entities, guid);
	return k == kh_end(&zrc_host->entities) ? ID_INVALID : (id_t)k;
}