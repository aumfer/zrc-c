#pragma once

#include <zrc.h>
#include <khash.h>
#include <guid.h>

KHASH_INIT(ehash, guid_t, char, 0, guid_hash_func, guid_eq_func)

typedef struct demo_world {
	id_t radiant, dire, player;
} demo_world_t;

typedef struct zrc_host {
	khash_t(ehash) *entities;
	demo_world_t demo_world; // todo move

	timer_t timer;
	moving_average_t tick_fps;
	double accumulator;
	unsigned frame;
} zrc_host_t;

void zrc_host_startup(zrc_host_t *, zrc_t *);
void zrc_host_shutdown(zrc_host_t *);

void zrc_host_tick(zrc_host_t *, zrc_t *);
void zrc_host_update(zrc_host_t *, zrc_t *);

id_t zrc_host_put(zrc_host_t *, guid_t);
id_t zrc_host_del(zrc_host_t *, guid_t);
id_t zrc_host_get(const zrc_host_t *, guid_t);

void demo_world_create(demo_world_t *, zrc_host_t *, zrc_t *);
void demo_world_delete(demo_world_t *);