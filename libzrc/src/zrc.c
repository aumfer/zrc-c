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
	zrc->time = stm_now();

	registry_startup(zrc);
	physics_startup(zrc);
	visual_startup(zrc);
	flight_startup(zrc);
	life_startup(zrc);
}
void zrc_shutdown(zrc_t *zrc) {
	life_shutdown(zrc);
	flight_shutdown(zrc);
	visual_shutdown(zrc);
	physics_shutdown(zrc);
	registry_shutdown(zrc);
}

void zrc_tick(zrc_t *zrc) {
	uint64_t time = stm_now();
	uint64_t dt = stm_diff(time, zrc->time);
	double dts = stm_sec(dt);
	zrc->time = time;

	moving_average_update(&zrc->fps, (float)dts);

	zrc->accumulator += dts;
	int frames = 0;
	while (zrc->accumulator >= TICK_RATE) {
		zrc->accumulator -= TICK_RATE;
		++frames;

		ZRC_UPDATE0(zrc, registry);
		ZRC_UPDATE1(zrc, flight);
		ZRC_UPDATE2(zrc, physics);
		ZRC_CLEAR(zrc, damage);
		ZRC_UPDATE1(zrc, life);
		ZRC_UPDATE1(zrc, visual);
	}

	if (frames > 1) {
		printf("stall %d frames\n", frames);
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

void flight_startup(zrc_t *zrc) {
	printf("flight %zu\n", sizeof(zrc->flight));
}
void flight_shutdown(zrc_t *zrc) {
}
void flight_create(zrc_t *zrc, id_t id, flight_t *flight) {

}
void flight_delete(zrc_t *zrc, id_t id, flight_t *flight) {

}
void flight_update(zrc_t *zrc, id_t id, flight_t *flight) {
}

void life_startup(zrc_t *zrc) {
	printf("life %zu\n", sizeof(zrc->life));
}
void life_shutdown(zrc_t *zrc) {
}
void life_create(zrc_t *zrc, id_t id, life_t *life) {

}
void life_delete(zrc_t *zrc, id_t id, life_t *life) {

}
void life_update(zrc_t *zrc, id_t id, life_t *life) {
	damage_t *damage;
	ZRC_RECEIVE(zrc, damage, id, damage, {
		life->health = max(0, life->health - damage->health);
	});
	if (life->health <= 0) {
		ZRC_DESPAWN(zrc, life, id);
	}
}