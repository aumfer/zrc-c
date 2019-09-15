#include <zrc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define SOKOL_IMPL
#include <sokol_time.h>
#include <stdarg.h>

registry_t zrc_components(int count, ...) {
	registry_t components = 0;
	va_list argp;
	va_start(argp, count);
	for (int i = 0; i < count; ++i) {
		registry_t next = va_arg(argp, registry_t);
		components |= next;
	}
	va_end(argp);
	return components;
}

void zrc_startup(zrc_t *zrc) {
	printf("zrc %zu\n", sizeof(zrc_t));

	flight_startup(zrc);
	physics_controller_startup(zrc);
	physics_startup(zrc);
	life_startup(zrc);
	caster_startup(zrc);
	ttl_startup(zrc);
	contact_damage_startup(zrc);
	locomotion_startup(zrc);
	seek_startup(zrc);
	sense_startup(zrc);

	zrc->ability[ABILITY_TUR_PROJ_ATTACK] = (ability_t) {
		.target_flags = ABILITY_TARGET_POINT,
		.range = 250,
		.cooldown = 2.0f / 3.0f / 2.0f,
		.channel = 1.0f / 3.0f / 2.0f,
		.mana = 1
	};
	zrc->ability[ABILITY_BLINK] = (ability_t) {
		.target_flags = ABILITY_TARGET_POINT,
		.range = 250,
		.cooldown = 7,
		.channel = 3,
		.mana = 10
	};
	zrc->ability[ABILITY_FIX_PROJ_ATTACK] = (ability_t) {
		.target_flags = ABILITY_TARGET_POINT,
		.range = 250,
		.cooldown = 2.0f / 3.0f,
		.channel = 1.0f / 3.0f,
		.mana = 5
	};
	zrc->ability[ABILITY_TARGET_NUKE] = (ability_t) {
		.target_flags = ABILITY_TARGET_UNIT,
		.range = 250,
		.cooldown = 3.0f,
		.channel = 2.0f,
		.mana = 25
	};

	timer_create(&zrc->timer);
}
void zrc_shutdown(zrc_t *zrc) {
	sense_shutdown(zrc);
	seek_shutdown(zrc);
	locomotion_shutdown(zrc);
	contact_damage_shutdown(zrc);
	ttl_shutdown(zrc);
	caster_shutdown(zrc);
	life_shutdown(zrc);
	physics_shutdown(zrc);
	physics_controller_shutdown(zrc);
	flight_shutdown(zrc);
}

void zrc_tick(zrc_t *zrc) {
	timer_update(&zrc->timer);

	double dts = stm_sec(zrc->timer.dt);
	moving_average_update(&zrc->fps, (float)dts);

	zrc->accumulator += dts;
	int frames = 0;
	while (zrc->accumulator >= TICK_RATE) {
		zrc->accumulator -= TICK_RATE;
		++zrc->frame;
		++frames;

		ZRC_UPDATE0(zrc, registry);

		ZRC_UPDATE1(zrc, flight);
		ZRC_UPDATE1(zrc, physics_controller);
		ZRC_UPDATE2(zrc, physics);
		ZRC_UPDATE1(zrc, life);
		ZRC_UPDATE0(zrc, visual);
		ZRC_UPDATE1(zrc, caster);
		ZRC_UPDATE1(zrc, ttl);
		ZRC_UPDATE1(zrc, contact_damage);
		ZRC_UPDATE1(zrc, locomotion);
		ZRC_UPDATE1(zrc, seek);
		ZRC_UPDATE1(zrc, sense);
	}

	if (frames > 1) {
		printf("stall %d frames\n", frames-1);
	}
}

void ttl_startup(zrc_t *zrc) {
	printf("ttl %zu\n", sizeof(zrc->ttl));
}
void ttl_shutdown(zrc_t *zrc) {

}
void ttl_create(zrc_t *zrc, id_t id, ttl_t *ttl) {
}
void ttl_delete(zrc_t *zrc, id_t id, ttl_t *ttl) {
}
void ttl_update(zrc_t *zrc, id_t id, ttl_t *ttl) {
	ttl->alive += TICK_RATE;
	if (ttl->alive >= ttl->ttl) {
		ZRC_DESPAWN_ALL(zrc, id);
	}
}

void sense_startup(zrc_t *zrc) {
	printf("sense %zu\n", sizeof(zrc->sense));
}
void sense_shutdown(zrc_t *zrc) {

}
void sense_create(zrc_t *zrc, id_t id, sense_t *sense) {

}
void sense_delete(zrc_t *zrc, id_t id, sense_t *sense) {

}
static void sense_bb_query(cpShape *shape, void *data) {
	sense_t *sense = data;
	id_t id = (id_t)cpShapeGetUserData(shape);
	assert(sense->num_entities < SENSE_MAX_ENTITIES);
	sense->entities[sense->num_entities++&SENSE_MASK_ENTITIES] = id;
}
void sense_update(zrc_t *zrc, id_t id, sense_t *sense) {
	physics_t *physics = ZRC_GET(zrc, physics, id);
	cpBB bounds = {
		.l = physics->position.x - physics->radius - sense->range,
		.r = physics->position.x + physics->radius + sense->range,
		.b = physics->position.y - physics->radius - sense->range,
		.t = physics->position.y + physics->radius + sense->range
	};
	sense->num_entities = 0;
	cpSpaceBBQuery(zrc->space, bounds, CP_SHAPE_FILTER_ALL, sense_bb_query, sense);
}