#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <limits.h>
#include <stdint.h>
#include <chipmunk/chipmunk.h>
#include <assert.h>

typedef uint16_t id_t;
#define ID_INVALID ((uint16_t)-1)

#define MAX_ENTITIES 16384
#define MAX_FRAMES 64
#define MASK_FRAMES (MAX_FRAMES-1)

#define TICK_RATE (1.0f/60.0f)

typedef enum zrc_component {
	zrc_registry = 0,
	zrc_physics = 1,
	zrc_visual = 2,
	zrc_flight = 4,
	zrc_life = 8
} zrc_component_t;

typedef uint16_t registry_t;

typedef struct physics {
	cpBodyType type;
	cpBitmask collide_flags;
	cpBitmask collide_mask;
	cpBitmask response_mask;
	float radius;

	float position[2];
	float angle;
	float velocity[2];
	float angular_velocity;

	cpBody *body;
	cpShape *shape;
} physics_t;

#define MAX_MOTIONS 2

typedef struct motion {
	float force[2];
	float torque;
} motion_t;

typedef struct visual {
	uint32_t color;
	uint32_t flags;
	float transform[4][4];
} visual_t;

typedef struct flight {
	float thrust[2];
	float turn;
} flight_t;

typedef struct life {
	float health, strength, constitution;
	float max_health, max_strength, max_constitution;
	float mana, focus, willpower;
	float max_mana, max_focus, max_willpower;
	float rage, serenity, temper;
	float max_rage, max_serentiy, max_temper;
} life_t;

#define MAX_DAMAGES 32
typedef struct damage {
	id_t from;
	float health;
} damage_t;

typedef struct zrc {
	// persistent
	registry_t registry[MAX_FRAMES][MAX_ENTITIES];
	physics_t physics[MAX_FRAMES][MAX_ENTITIES];
	visual_t visual[MAX_FRAMES][MAX_ENTITIES];
	flight_t flight[MAX_FRAMES][MAX_ENTITIES];
	life_t life[MAX_FRAMES][MAX_ENTITIES];

	// transient
	uint8_t num_damage[MAX_FRAMES][MAX_ENTITIES];
	damage_t damage[MAX_FRAMES][MAX_ENTITIES][MAX_DAMAGES];
	uint8_t num_motion[MAX_FRAMES][MAX_ENTITIES];
	motion_t motion[MAX_FRAMES][MAX_ENTITIES][MAX_MOTIONS];

	struct {
		unsigned registry;
		unsigned physics;
		unsigned visual;
		unsigned flight;
		unsigned life;
		unsigned damage;
		unsigned motion;
	} frames;
	unsigned frame;
	cpSpace *space;
	cpCollisionHandler *collision_handler;
} zrc_t;

#define ZRC_PAST_FRAME(zrc, n) (((zrc)->frame - (n)) & MASK_FRAMES)
#define ZRC_PREV_FRAME(zrc) ZRC_PAST_FRAME(zrc, 2)
#define ZRC_READ_FRAME(zrc) ZRC_PAST_FRAME(zrc, 1)
#define ZRC_WRITE_FRAME(zrc) ZRC_PAST_FRAME(zrc, 0)

#define ZRC_HAD(zrc, name, id) (((zrc)->registry[ZRC_PREV_FRAME(zrc)][id] & zrc_##name##) == zrc_##name##)
#define ZRC_HAS(zrc, name, id) (((zrc)->registry[ZRC_READ_FRAME(zrc)][id] & zrc_##name##) == zrc_##name##)

#define ZRC_GET_READ(zrc, name, id) (&(zrc)->##name##[ZRC_READ_FRAME(zrc)][id])
#define ZRC_GET_WRITE(zrc, name, id) (&(zrc)->##name##[ZRC_WRITE_FRAME(zrc)][id])
#define ZRC_GET(zrc, name, id) (ZRC_HAS(zrc, name, id) ? ZRC_GET_READ(zrc, name, id) : 0)

#define ZRC_RECEIVE(zrc, name, id, var, code) \
	for (int i = 0; i < *ZRC_GET_READ(zrc, num_##name##, id); ++i) { \
		(var) = (ZRC_GET_READ(zrc, name, id)[i]); \
		code; \
	}

#define ZRC_SEND(zrc, name, id, val) *ZRC_GET_WRITE(zrc, name, id)[(*ZRC_GET_WRITE(zrc, num_##name##, id))++] = *(val)

#define ZRC_UPDATE0(zrc, name) \
	for (int i = 0; i < MAX_ENTITIES; ++i) { \
		*ZRC_GET_WRITE(zrc, num_##name##, i) = 0; \
	}

#define ZRC_UPDATE1(zrc, name) \
	for (int i = 0; i < MAX_ENTITIES; ++i) { \
		if (!ZRC_HAS(zrc, name, i)) { \
			if (ZRC_HAD(zrc, name, i)) { \
				##name##_t *prev = ZRC_GET_READ(zrc, name, i); \
				##name##_delete((zrc), i, prev); \
			} \
		} else { \
			##name##_t *prev = ZRC_GET_READ(zrc, name, i); \
			##name##_t *next = ZRC_GET_WRITE(zrc, name, i); \
			if (!ZRC_HAD(zrc, name, i)) { \
				##name##_create((zrc), i, prev); \
			} else { \
				##name##_update((zrc), i, prev); \
			} \
			*next = *prev; \
		} \
	}

#define ZRC_UPDATE2(zrc, name) \
	for (int i = 0; i < MAX_ENTITIES; ++i) { \
		if (!ZRC_HAS(zrc, name, i)) { \
			if (ZRC_HAD(zrc, name, i)) { \
				##name##_t *prev = ZRC_GET_READ(zrc, name, i); \
				##name##_delete((zrc), i, prev); \
			} \
		} else { \
			##name##_t *prev = ZRC_GET_READ(zrc, name, i); \
			if (!ZRC_HAD(zrc, name, i)) { \
				##name##_create((zrc), i, prev); \
			} \
			##name##_begin((zrc), i, prev); \
		} \
	} \
	##name##_update((zrc)); \
	for (int i = 0; i < MAX_ENTITIES; ++i) { \
		if (ZRC_HAS(zrc, name, i)) { \
			##name##_t *prev = ZRC_GET_READ(zrc, name, i); \
			##name##_t *next = ZRC_GET_WRITE(zrc, name, i); \
			##name##_end((zrc), i, prev); \
			*next = *prev; \
		} \
	}

#define ZRC_SPAWN(zrc, name, id, value) do { \
		assert(!ZRC_HAS(zrc, name, id)); \
		*ZRC_GET_WRITE(zrc, registry, id) |= zrc_##name##; \
		*ZRC_GET_WRITE(zrc, name, id) = *(value); \
	} while (0)

#define ZRC_DESPAWN(zrc, name, id) (*ZRC_GET_WRITE(zrc, registry, id) &= ~zrc_##name##)

void registry_create(zrc_t *, id_t, registry_t *);
void registry_delete(zrc_t *, id_t, registry_t *);
void registry_update(zrc_t *, id_t, registry_t *);

void physics_startup(zrc_t *);
void physics_shutdown(zrc_t *);
void physics_create(zrc_t *, id_t, physics_t *);
void physics_delete(zrc_t *, id_t, physics_t *);
void physics_begin(zrc_t *, id_t, physics_t *);
void physics_update(zrc_t *);
void physics_end(zrc_t *, id_t, physics_t *);
id_t physics_query_ray(zrc_t *, float start[2], float end[2], float radius);
id_t physics_query_point(zrc_t *, float point[2], float radius);

void visual_create(zrc_t *, id_t, visual_t *);
void visual_delete(zrc_t *, id_t, visual_t *);
void visual_update(zrc_t *, id_t, visual_t *);

void flight_create(zrc_t *, id_t, flight_t *);
void flight_delete(zrc_t *, id_t, flight_t *);
void flight_update(zrc_t *, id_t, flight_t *);

#ifdef __cplusplus
}
#endif