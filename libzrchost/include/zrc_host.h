#pragma once

#include <zrc.h>
#include <khash.h>
#include <guid.h>
#include <tf_brain.h>

KHASH_INIT(ehash, guid_t, char, 0, guid_hash_func, guid_eq_func)

#define TEAM_RADIANT 1
#define TEAM_DIRE 2
#define TEAM_OTHER 4

#define NUM_TEST_ENTITIES 16
#define SPAWN_SIZE (WORLD_SIZE/2)

typedef struct demo_world {
	id_t test_entities[NUM_TEST_ENTITIES];
} demo_world_t;

typedef struct zrc_host {
	khash_t(ehash) *entities;
	demo_world_t demo_world; // todo move

	tf_brain_t locomotion_brain;
	tf_brain_t sense_brain;

	unsigned frame;
} zrc_host_t;

void zrc_host_startup(zrc_host_t *, zrc_t *);
void zrc_host_shutdown(zrc_host_t *);

void zrc_host_update(zrc_host_t *, zrc_t *);

id_t zrc_host_put(zrc_host_t *, guid_t);
id_t zrc_host_del(zrc_host_t *, guid_t);
id_t zrc_host_get(const zrc_host_t *, guid_t);

void demo_world_create(demo_world_t *, zrc_host_t *, zrc_t *);
void demo_world_delete(demo_world_t *);