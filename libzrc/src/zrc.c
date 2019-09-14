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

	physics_startup(zrc);
	flight_startup(zrc);
	life_startup(zrc);
	ttl_startup(zrc);

	zrc->ability[ABILITY_PROJATTACK] = (ability_t) {
		.target_flags = ABILITY_TARGET_POINT,
		.range = 1024,
		.cooldown = 2.0f/3.0f,
		.channel = 1.0f/3.0f,
		.mana = 1
	};
	zrc->ability[ABILITY_BLINK] = (ability_t) {
		.target_flags = ABILITY_TARGET_POINT,
		.range = 1024,
		.cooldown = 7,
		.channel = 3,
		.mana = 10
	};

	timer_create(&zrc->timer);
}
void zrc_shutdown(zrc_t *zrc) {
	ttl_shutdown(zrc);
	life_shutdown(zrc);
	flight_shutdown(zrc);
	physics_shutdown(zrc);
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
		ZRC_UPDATE2(zrc, physics);
		ZRC_UPDATE1(zrc, physics_controller);
		ZRC_UPDATE1(zrc, life);
		ZRC_UPDATE0(zrc, visual);
		ZRC_UPDATE1(zrc, caster);
		ZRC_UPDATE1(zrc, ttl);
	}

	if (frames > 1) {
		printf("stall %d frames\n", frames-1);
	}
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
	for (int i = 0; i < CASTER_MAX_CASTS; ++i) {
		cast_t *cast = &caster->casts[i];
		const ability_t *ability = &zrc->ability[cast->ability];
		if ((cast->cast_flags & CAST_ISCAST) == CAST_ISCAST) {
			if (ability->cast) {
				ability->cast(zrc, ability, id, &cast->target);
			}
			cast->uptime += TICK_RATE;
			if (cast->uptime >= ability->channel) {
				cast->cast_flags &= ~CAST_ISCAST;
				cast->uptime -= ability->channel;
				cast->downtime += cast->uptime;
				cast->uptime = 0;
			}
		} else {
			cast->downtime += TICK_RATE;
			if ((cast->cast_flags & CAST_WANTCAST) == CAST_WANTCAST) {
				if (cast->downtime >= ability->cooldown) {
					cast->downtime -= ability->cooldown;
					cast->uptime += cast->downtime;
					cast->downtime = 0;
					if (ability->cast) {
						ability->cast(zrc, ability, id, &cast->target);
					}
				}
			}
		}
	}
}

void ttl_startup(zrc_t *zrc) {
	printf("ttl %zu\n", sizeof(zrc->ttl));
}
void ttl_shutdown(zrc_t *zrc) {

}
void ttl_create(zrc_t *zrc, id_t id, ttl_t *ttl) {
	puts("hi");
}
void ttl_delete(zrc_t *zrc, id_t id, ttl_t *ttl) {

}
void ttl_update(zrc_t *zrc, id_t id, ttl_t *ttl) {
	ttl->alive += TICK_RATE;
	if (ttl->alive >= ttl->ttl) {
		ZRC_DESPAWN_ALL(zrc, id);
	}
}