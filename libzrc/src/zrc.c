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

	stm_setup();

	registry_startup(zrc);
	physics_startup(zrc);
	visual_startup(zrc);
	flight_startup(zrc);
	life_startup(zrc);

	zrc->ability[1] = (ability_t) {
		.name = "autoattack",
		.target_flags = ABILITY_TARGET_POINT,
		.range = 1024,
		.cooldown = 2.0f/3.0f,
		.channel = 1.0f/3.0f,
		.mana = 1
	};

	timer_create(&zrc->timer);
}
void zrc_shutdown(zrc_t *zrc) {
	life_shutdown(zrc);
	flight_shutdown(zrc);
	visual_shutdown(zrc);
	physics_shutdown(zrc);
	registry_shutdown(zrc);
}

void zrc_tick(zrc_t *zrc) {
	timer_update(&zrc->timer);

	double dts = stm_sec(zrc->timer.dt);
	moving_average_update(&zrc->fps, (float)dts);

	zrc->accumulator += dts;
	int frames = 0;
	while (zrc->accumulator >= TICK_RATE) {
		zrc->accumulator -= TICK_RATE;
		++frames;

		ZRC_UPDATE0(zrc, registry);
		ZRC_UPDATE1(zrc, flight);
		ZRC_UPDATE2(zrc, physics);
		ZRC_UPDATE1(zrc, physics_controller);
		ZRC_CLEAR(zrc, damage);
		ZRC_UPDATE1(zrc, life);
		ZRC_UPDATE1(zrc, visual);
		ZRC_UPDATE1(zrc, caster);

		++zrc->frame;
		zrc->time = stm_now();
	}

	if (frames > 1) {
		printf("%u stall %d frames\n", zrc->frame, frames);
	}
}

void registry_startup(zrc_t *zrc) {
	printf("registry %zu\n", sizeof(zrc->registry));
}
void registry_shutdown(zrc_t *zrc) {

}
void registry_create(zrc_t *zrc, id_t id, registry_t *registry) {

}
void registry_delete(zrc_t *zrc, id_t id, registry_t *registry) {

}
void registry_update(zrc_t *zrc, id_t id, registry_t *registry) {
}

void visual_startup(zrc_t *zrc) {
	printf("visual %zu\n", sizeof(zrc->visual));
}
void visual_shutdown(zrc_t *zrc) {
}
void visual_create(zrc_t *zrc, id_t id, visual_t *visual) {

}
void visual_delete(zrc_t *zrc, id_t id, visual_t *visual) {

}
void visual_update(zrc_t *zrc, id_t id, visual_t *visual) {
}

void caster_startup(zrc_t *zrc) {
	printf("caster %zu\n", sizeof(zrc->caster));
}
void caster_shutdown(zrc_t *zrc) {
}
void caster_create(zrc_t *zrc, id_t id, caster_t *caster) {

}
void caster_delete(zrc_t *zrc, id_t id, caster_t *caster) {

}
void caster_update(zrc_t *zrc, id_t id, caster_t *caster) {
}