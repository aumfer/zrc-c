#ifndef _ZRC_H_
#define _ZRC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <limits.h>
#include <stdint.h>
#include <chipmunk/chipmunk.h>
#include <assert.h>
#include <sokol_time.h>
#include <moving_average.h>
#include <timer.h>
#include <color.h>

typedef uint16_t id_t;
#define ID_INVALID ((id_t)-1)

#define MAX_ENTITIES 16384
#define MAX_FRAMES 64
#define MASK_FRAMES (MAX_FRAMES-1)

#define MAX_MESSAGES 64
#define MASK_MESSAGES (MAX_MESSAGES-1)

#define TICK_RATE (1.0f/60.0f)

#define randf() ((float)rand() / RAND_MAX)

typedef struct zrc zrc_t;

typedef enum zrc_component {
	zrc_registry,
	zrc_physics,
	zrc_visual,
	zrc_flight,
	zrc_life,
	zrc_physics_controller,
	zrc_caster,
	zrc_ttl,
	zrc_contact_damage,
	zrc_component_count
} zrc_component_t;

typedef uint16_t registry_t;

static_assert(zrc_component_count <= sizeof(registry_t)*8, "too many components");

typedef struct physics {
	cpBodyType type;
	cpBitmask collide_flags;
	cpBitmask collide_mask;
	cpBitmask response_mask;
	float max_speed;
	float max_spin;

	float radius;
	float angle;
	cpVect position;
	cpVect velocity;
	float angular_velocity;
	float torque;
	cpVect force;

	cpBody *body;
	cpShape *shape;

	unsigned num_forces;
} physics_t;

typedef struct physics_force {
	cpVect force;
	float torque;
} physics_force_t;

typedef struct physics_controller {
	cpBody *body;
	cpConstraint *pivot;
	cpConstraint *gear;

	unsigned num_velocities;
} physics_controller_t;

typedef struct physics_controller_velocity {
	cpVect velocity;
	float angular_velocity;
} physics_controller_velocity_t;

typedef struct visual {
	uint32_t color;
	uint32_t flags;
	float size[2];
	float position[2];
	float angle;
} visual_t;

typedef struct flight {
	float max_thrust;
	float max_turn;

	float thrust[2];
	float turn;
} flight_t;

typedef struct life {
	float health, max_health, strength, constitution;
	float mana, max_mana, focus, willpower;
	float rage, max_rage, serenity, temper;

	unsigned num_damages;
} life_t;

typedef struct damage {
	id_t from;
	float health;
} damage_t;

typedef enum ability_target_flags {
	ABILITY_TARGET_NONE = 0,
	ABILITY_TARGET_UNIT = 1,
	ABILITY_TARGET_POINT = 2,
	ABILITY_TARGET_FRIEND = 4 | ABILITY_TARGET_UNIT,
	ABILITY_TARGET_ENEMY = 8 | ABILITY_TARGET_UNIT,
	ABILITY_TARGET_SELF = 16 | ABILITY_TARGET_UNIT
} ability_target_flags_t;

typedef union ability_target {
	id_t unit;
	float point[2];
} ability_target_t;

typedef struct ability ability_t;

typedef void(*ablity_cast_t)(zrc_t *, const ability_t *, id_t, const ability_target_t *);

typedef enum ability_id {
	ABILITY_NONE,
	ABILITY_TUR_PROJ_ATTACK,
	ABILITY_BLINK,
	ABILITY_FIX_PROJ_ATTACK,
	ABILITY_TARGET_NUKE,
	ABILITY_COUNT
} ability_id_t;

typedef struct ability {
	ability_target_flags_t target_flags;
	ablity_cast_t cast;
	float range;
	float cooldown;
	float channel;
	float mana;
	float rage;
} ability_t;

typedef enum cast_flags {
	CAST_NONE,
	CAST_WANTCAST,
	CAST_ISCAST
} cast_flags_t;

typedef struct caster_ability {
	ability_id_t ability;
	float uptime;
	float downtime;

	ability_target_t target;
	cast_flags_t cast_flags;
} caster_ability_t;

typedef uint8_t caster_ability_id_t;

typedef struct cast {
	caster_ability_id_t caster_ability;
	ability_target_t target;
	cast_flags_t cast_flags;
} cast_t;

#define CASTER_MAX_ABLITIES 8
typedef struct caster {
	caster_ability_t abilities[CASTER_MAX_ABLITIES];

	unsigned num_casts;
} caster_t;

typedef struct ttl {
	float ttl;
	float alive;
} ttl_t;

typedef enum contact_damage_flags {
	CONTACT_DAMAGE_NONE,
	CONTACT_DAMAGE_HAS_HIT = 1,
	CONTACT_DAMAGE_ONE_HIT = 2,
	CONTACT_DAMAGE_DESPAWN_ON_HIT = 4
} contact_damage_flags_t;

typedef struct contact_damage {
	damage_t damage;
	contact_damage_flags_t flags;
	id_t onhit_id;
	visual_t visual;
	ttl_t ttl;
} contact_damage_t;

typedef struct zrc {
	// static
	ability_t ability[ABILITY_COUNT];

	// persistent
	registry_t registry[MAX_FRAMES][MAX_ENTITIES];
	physics_t physics[MAX_FRAMES][MAX_ENTITIES];
	physics_controller_t physics_controller[MAX_FRAMES][MAX_ENTITIES];
	visual_t visual[MAX_FRAMES][MAX_ENTITIES];
	flight_t flight[MAX_FRAMES][MAX_ENTITIES];
	life_t life[MAX_FRAMES][MAX_ENTITIES];
	caster_t caster[MAX_FRAMES][MAX_ENTITIES];
	ttl_t ttl[MAX_FRAMES][MAX_ENTITIES];
	contact_damage_t contact_damage[MAX_FRAMES][MAX_ENTITIES];

	// transient
	unsigned num_damage[MAX_ENTITIES];
	damage_t damage[MAX_ENTITIES][MAX_MESSAGES];
	unsigned num_physics_force[MAX_ENTITIES];
	physics_force_t physics_force[MAX_ENTITIES][MAX_MESSAGES];
	unsigned num_physics_controller_velocity[MAX_ENTITIES];
	physics_controller_velocity_t physics_controller_velocity[MAX_ENTITIES][MAX_MESSAGES];
	unsigned num_cast[MAX_ENTITIES];
	cast_t cast[MAX_ENTITIES][MAX_MESSAGES];

	unsigned frame;
	uint64_t times[zrc_component_count];
	timer_t timer;
	double accumulator;
	moving_average_t fps;

	void *user;
	cpSpace *space;
	cpCollisionHandler *collision_handler;
} zrc_t;

registry_t zrc_components(int count, ...);

#define ZRC_PAST_FRAME(zrc, name, n) (((zrc)->frame - (n)) & MASK_FRAMES)
#define ZRC_PREV_FRAME(zrc, name) ZRC_PAST_FRAME(zrc, name, 2)
#define ZRC_READ_FRAME(zrc, name) ZRC_PAST_FRAME(zrc, name, 1)
#define ZRC_WRITE_FRAME(zrc, name) ZRC_PAST_FRAME(zrc, name, 0)
#define ZRC_NEXT_FRAME(zrc, name) ZRC_PAST_FRAME(zrc, name, -1)

#define ZRC_HAD_PAST(zrc, name, id, n) (((zrc)->registry[ZRC_PAST_FRAME(zrc, name, n)][id] & (1<<zrc_##name##)) == (1<<zrc_##name##))
#define ZRC_HAD(zrc, name, id) (((zrc)->registry[ZRC_PREV_FRAME(zrc, name)][id] & (1<<zrc_##name##)) == (1<<zrc_##name##))
#define ZRC_HAS(zrc, name, id) (((zrc)->registry[ZRC_READ_FRAME(zrc, name)][id] & (1<<zrc_##name##)) == (1<<zrc_##name##))

#define ZRC_GET_PAST(zrc, name, id, n) (&(zrc)->##name##[ZRC_PAST_FRAME(zrc, name, (n))][id])
#define ZRC_GET_PREV(zrc, name, id) (&(zrc)->##name##[ZRC_PREV_FRAME(zrc, name)][id])
#define ZRC_GET_READ(zrc, name, id) (&(zrc)->##name##[ZRC_READ_FRAME(zrc, name)][id])
#define ZRC_GET_WRITE(zrc, name, id) (&(zrc)->##name##[ZRC_WRITE_FRAME(zrc, name)][id])
#define ZRC_GET_NEXT(zrc, name, id) (&(zrc)->##name##[ZRC_NEXT_FRAME(zrc, name)][id])
#define ZRC_GET(zrc, name, id) (ZRC_HAS(zrc, name, id) ? ZRC_GET_READ(zrc, name, id) : 0)

#define ZRC_RECEIVE(zrc, name, id, start, var, code) \
	for (unsigned i = (*(start)); i < (zrc)->num_##name##[id]; ++i) { \
		(var) = &(zrc)->##name##[id][i&MASK_MESSAGES]; \
		code; \
		++(*(start)); \
	}

#define ZRC_SEND(zrc, name, id, val) (zrc)->##name##[id][(zrc)->num_##name##[id]++&MASK_MESSAGES] = *(val)

// components with no behavior (pure state)
#define ZRC_UPDATE0(zrc, name) do { \
		uint64_t start = stm_now(); \
		for (int i = 0; i < MAX_ENTITIES; ++i) { \
			/*if (ZRC_HAS(zrc, name, i))*/ { \
				##name##_t *prev = ZRC_GET_READ(zrc, name, i); \
				##name##_t *next = ZRC_GET_WRITE(zrc, name, i); \
				*next = *prev; \
			} \
		} \
		uint64_t end = stm_now(); \
		(zrc)->times[zrc_##name##] = stm_diff(end, start); \
	} while (0)

// components with no shared state
#define ZRC_UPDATE1(zrc, name) do { \
		uint64_t start = stm_now(); \
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
		} \
		uint64_t end = stm_now(); \
		(zrc)->times[zrc_##name##] = stm_diff(end, start); \
	} while (0)

// components with shared state
#define ZRC_UPDATE2(zrc, name) do { \
		uint64_t start = stm_now(); \
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
		} \
		uint64_t end = stm_now(); \
		(zrc)->times[zrc_##name##] = stm_diff(end, start); \
	} while (0)

#define ZRC_SPAWN(zrc, name, id, value) do { \
		assert(!ZRC_HAS(zrc, name, id)); \
		*ZRC_GET_WRITE(zrc, registry, id) |= ((1<<zrc_##name##) | (1<<zrc_registry)); \
		*ZRC_GET_WRITE(zrc, name, id) = *(value); \
	} while (0)

#define ZRC_DESPAWN(zrc, name, id) (*ZRC_GET_WRITE(zrc, registry, id) &= ~(1<<zrc_##name##))
#define ZRC_DESPAWN_ALL(zrc, id) (*ZRC_GET_WRITE(zrc, registry, id) = 0)

#define ZRC_DECLARE_COMPONENT(name) \
void zrc_##name##_startup(zrc_t *);\
void zrc_##name##_shutdown(zrc_t *);\
void zrc_##name##_create(zrc_t *, id_t, ##name##_t *);\
void zrc_##name##_delete(zrc_t *, id_t, ##name##_t *);\
void zrc_##name##_update(zrc_t *, id_t, ##name##_t *);\

#define ZRC_EMPTY_COMPONENT(name) \
void zrc_##name##_startup(zrc_t *zrc) { \
	printf("##name## %zu\n", sizeof(zrc->##name##)); \
} \
void zrc_##name##_shutdown(zrc_t *zrc) { \
} \
void zrc_##name##_create(zrc_t *zrc, id_t id, ##name##_t *##name##) { \
} \
void zrc_##name##_delete(zrc_t *zrc, id_t id, ##name##_t *##name##) { \
} \
void zrc_##name##_update(zrc_t *zrc, id_t id, ##name##_t *##name##) { \
} \

void zrc_startup(zrc_t *);
void zrc_shutdown(zrc_t *);
void zrc_tick(zrc_t *);

void physics_startup(zrc_t *);
void physics_shutdown(zrc_t *);
void physics_create(zrc_t *, id_t, physics_t *);
void physics_delete(zrc_t *, id_t, physics_t *);
void physics_begin(zrc_t *, id_t, physics_t *);
void physics_update(zrc_t *);
void physics_end(zrc_t *, id_t, physics_t *);
id_t physics_query_ray(zrc_t *, float start[2], float end[2], float radius);
id_t physics_query_point(zrc_t *, float point[2], float radius);

void flight_startup(zrc_t *);
void flight_shutdown(zrc_t *);
void flight_create(zrc_t *, id_t, flight_t *);
void flight_delete(zrc_t *, id_t, flight_t *);
void flight_update(zrc_t *, id_t, flight_t *);

void life_startup(zrc_t *);
void life_shutdown(zrc_t *);
void life_create(zrc_t *, id_t, life_t *);
void life_delete(zrc_t *, id_t, life_t *);
void life_update(zrc_t *, id_t, life_t *);

void physics_controller_startup(zrc_t *);
void physics_controller_shutdown(zrc_t *);
void physics_controller_create(zrc_t *, id_t, physics_controller_t *);
void physics_controller_delete(zrc_t *, id_t, physics_controller_t *);
void physics_controller_update(zrc_t *, id_t, physics_controller_t *);

void caster_startup(zrc_t *);
void caster_shutdown(zrc_t *);
void caster_create(zrc_t *, id_t, caster_t *);
void caster_delete(zrc_t *, id_t, caster_t *);
void caster_update(zrc_t *, id_t, caster_t *);

void ttl_startup(zrc_t *);
void ttl_shutdown(zrc_t *);
void ttl_create(zrc_t *, id_t, ttl_t *);
void ttl_delete(zrc_t *, id_t, ttl_t *);
void ttl_update(zrc_t *, id_t, ttl_t *);

void contact_damage_startup(zrc_t *);
void contact_damage_shutdown(zrc_t *);
void contact_damage_create(zrc_t *, id_t, contact_damage_t *);
void contact_damage_delete(zrc_t *, id_t, contact_damage_t *);
void contact_damage_update(zrc_t *, id_t, contact_damage_t *);

#ifdef __cplusplus
}
#endif

#endif