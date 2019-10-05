#ifndef _ZRC_H_
#define _ZRC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <limits.h>
#include <stdint.h>
#pragma warning(push)
#pragma warning(disable:4244)
#include <chipmunk/chipmunk.h>
#pragma warning(pop)
#include <assert.h>
#include <sokol_time.h>
#include <moving_average.h>
#include <timer.h>
#include <color.h>
#undef NDEBUG // for assert
#include <assert.h>
#include <zmath.h>

typedef uint16_t id_t;
#define ID_INVALID ((id_t)-1)

#define MAX_ENTITIES (16384/1)
#define MASK_ENTITIES (MAX_ENTITIES-1)
#define MAX_FRAMES (64/1)
#define MASK_FRAMES (MAX_FRAMES-1)

#define TICK_RATE (1.0f/60.0f)
#define WORLD_SIZE 2048
#define WORLD_HALF (WORLD_SIZE/2)
#define MAP_SCALE 16
#define WORLD_BB cpBBNew(-WORLD_HALF, -WORLD_HALF, WORLD_HALF, WORLD_HALF)
#define WORLD_INVALID cpv(NAN, NAN)

#define randf() ((float)rand() / RAND_MAX)
#define randfs() (snorm(randf()))

#define zrc_assert assert

typedef struct zrc zrc_t;

typedef uint8_t send_t;
typedef uint8_t recv_t;

typedef uint16_t registry_t;

#define SHIP_DAMPING 0.1f

typedef struct physics {
	cpBodyType type;
	cpBitmask collide_flags;
	cpBitmask collide_mask;
	cpBitmask response_mask;
	float max_speed;
	float max_spin;
	float damping;

	float radius;
	float mass;
	float moment;
	float angle;
	cpVect position;
	cpVect velocity;
	float angular_velocity;
	float torque;
	cpVect force;

	cpVect front;

	cpBody *body;
	cpShape *shape;

	recv_t recv_force;
} physics_t;

#define max_physics_force_messages 8

typedef struct physics_force {
	cpVect force;
	float torque;
	float damp;
} physics_force_t;

typedef struct physics_controller {
	cpBody *body;
	cpConstraint *pivot;
	cpConstraint *gear;

	recv_t recv_velocity;
} physics_controller_t;

#define max_physics_controller_velocity_messages 4

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

#define max_flight_thrust_messages 4

typedef struct flight_thrust {
	cpVect thrust;
	float turn;
	float damp;
} flight_thrust_t;

typedef struct flight {
	float max_thrust;
	float max_turn;

	float thrust_control_rate;
	float turn_control_rate;

	cpVect thrust;
	float turn;
	float damp;

	recv_t recv_thrust;
} flight_t;

typedef struct life {
	float health, max_health, strength, constitution;
	float mana, max_mana, focus, willpower;
	float rage, max_rage, serenity, temper;

	recv_t recv_damage;
} life_t;

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

typedef enum ability_id {
	ABILITY_INVALID = -1,
	ABILITY_NONE,
	ABILITY_TUR_PROJ_ATTACK,
	ABILITY_BLINK,
	ABILITY_FIX_PROJ_ATTACK,
	ABILITY_TARGET_NUKE,
	ABILITY_COUNT
} ability_id_t;

typedef struct ability ability_t;

typedef void(*ablity_cast_t)(zrc_t *, ability_id_t, id_t, const ability_target_t *);

typedef struct ability {
	ability_target_flags_t target_flags;
	ablity_cast_t cast;
	float range;
	float cooldown;
	float channel;
	float mana;
	float rage;
} ability_t;

#define max_damage_messages 64

typedef struct damage {
	id_t from;
	ability_id_t ability;
	float health;
} damage_t;

#define max_damage_dealt_messages 64

typedef struct damage_dealt {
	id_t to;
	ability_id_t ability;
	float health;
} damage_dealt_t;

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
#define CASTER_ABILITY_INVALID ((caster_ability_id_t)-1)

#define max_cast_messages 8

typedef struct cast {
	caster_ability_id_t caster_ability;
	ability_target_t target;
	cast_flags_t cast_flags;
} cast_t;

#define CASTER_MAX_ABLITIES ABILITY_COUNT
typedef struct caster {
	caster_ability_t abilities[CASTER_MAX_ABLITIES];

	recv_t recv_cast;
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

#define max_locomotion_behavior_messages 16

typedef double(*locomotion_behavior_t)(const zrc_t *, id_t, cpVect point, cpVect front);

#define NUM_LOCOMOTION_THRUSTS 5
#define NUM_LOCOMOTION_TURNS 7

typedef struct locomotion {
	recv_t recv_locomotion_behavior;
} locomotion_t;

#define max_seek_to_messages 1

typedef struct seek_to {
	cpVect point;
} seek_to_t;

typedef struct seek {
	cpVect point;

	recv_t recv_seek_to;
} seek_t;

#define max_align_to_messages 1

typedef struct align_to {
	float angle;
} align_to_t;

typedef struct align {
	float angle;

	recv_t recv_align_to;
} align_t;

#define SENSE_MAX_ENTITIES 64
typedef struct sense {
	float range;

	id_t entities[SENSE_MAX_ENTITIES];
	int num_entities;
} sense_t;

typedef struct relate_to {
	id_t id;
	float value;
} relate_to_t;

#define RELATE_MAX_TO 1 // temp
typedef struct relate {
	relate_to_t to[RELATE_MAX_TO];
	int num_relates;
} relate_t;

typedef struct relationship {
	id_t from;
	id_t to;
	float amount;
} relationship_t;

#define max_rl_act_messages 1

typedef struct rl_act {
	float thrust[2];
	float turn;
	float damp;
} rl_act_t;

#define RL_ACT_LENGTH (sizeof(rl_act_t)/sizeof(float))

#define max_rl_obs_messages 1

typedef struct rl_obs {
	float values[8][8];
	float current;
} rl_obs_t;

#define RL_OBS_LENGTH (sizeof(rl_obs_t)/sizeof(float))

typedef struct rl {
	int done;
	float total_reward;
	float reward;

	recv_t recv_rl_act;
	recv_t recv_rl_obs;
	recv_t recv_locomotion_behavior;
} rl_t;

typedef uint32_t team_t;

#define max_got_kill_messages 64

typedef enum zrc_component {
	zrc_component_registry,
	zrc_component_physics,
	zrc_component_visual,
	zrc_component_flight,
	zrc_component_life,
	zrc_component_physics_controller,
	zrc_component_caster,
	zrc_component_ttl,
	zrc_component_contact_damage,
	zrc_component_locomotion,
	zrc_component_seek,
	zrc_component_align,
	zrc_component_sense,
	//zrc_component_relate,
	zrc_component_team,
	zrc_component_rl,
	zrc_component_count
} zrc_component_t;
static_assert(zrc_component_count <= sizeof(registry_t) * 8, "too many components");

// don't feel like renaming these right now so i'll just typedef them
typedef registry_t zrc_registry_t;
typedef physics_t zrc_physics_t;
typedef visual_t zrc_visual_t;
typedef flight_t zrc_flight_t;
typedef life_t zrc_life_t;
typedef physics_controller_t zrc_physics_controller_t;
typedef caster_t zrc_caster_t;
typedef ttl_t zrc_ttl_t;
typedef contact_damage_t zrc_contact_damage_t;
typedef locomotion_t zrc_locomotion_t;
typedef seek_t zrc_seek_t;
typedef align_t zrc_align_t;
typedef sense_t zrc_sense_t;
typedef team_t zrc_team_t;
typedef rl_t zrc_rl_t;

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
	locomotion_t locomotion[MAX_FRAMES][MAX_ENTITIES];
	seek_t seek[MAX_FRAMES][MAX_ENTITIES];
	align_t align[MAX_FRAMES][MAX_ENTITIES];
	sense_t sense[MAX_FRAMES][MAX_ENTITIES];
	//relate_t relate[MAX_FRAMES][MAX_ENTITIES];
	rl_t rl[MAX_FRAMES][MAX_ENTITIES];
	team_t team[MAX_FRAMES][MAX_ENTITIES];

	// transient // todo volatile?
	send_t send_damage[MAX_ENTITIES];
	damage_t damage[MAX_ENTITIES][max_damage_messages];
	send_t send_physics_force[MAX_ENTITIES];
	physics_force_t physics_force[MAX_ENTITIES][max_physics_force_messages];
	send_t send_physics_controller_velocity[MAX_ENTITIES];
	physics_controller_velocity_t physics_controller_velocity[MAX_ENTITIES][max_physics_controller_velocity_messages];
	send_t send_cast[MAX_ENTITIES];
	cast_t cast[MAX_ENTITIES][max_cast_messages];
	send_t send_locomotion_behavior[MAX_ENTITIES];
	locomotion_behavior_t locomotion_behavior[MAX_ENTITIES][max_locomotion_behavior_messages];
	send_t send_flight_thrust[MAX_ENTITIES];
	flight_thrust_t flight_thrust[MAX_ENTITIES][max_flight_thrust_messages];
	send_t send_seek_to[MAX_ENTITIES];
	seek_to_t seek_to[MAX_ENTITIES][max_seek_to_messages];
	send_t send_align_to[MAX_ENTITIES];
	align_to_t align_to[MAX_ENTITIES][max_align_to_messages];
	send_t send_damage_dealt[MAX_ENTITIES];
	damage_dealt_t damage_dealt[MAX_ENTITIES][max_damage_dealt_messages];
	send_t send_got_kill[MAX_ENTITIES];
	id_t got_kill[MAX_ENTITIES][max_got_kill_messages];
	send_t send_rl_act[MAX_ENTITIES];
	rl_act_t rl_act[MAX_ENTITIES][max_rl_act_messages];
	send_t send_rl_obs[MAX_ENTITIES];
	rl_obs_t rl_obs[MAX_ENTITIES][max_rl_obs_messages];

	unsigned frame;
	uint64_t times[zrc_component_count];
	moving_average_t update_fps;

	void *user;
	cpSpace *space;
	cpCollisionHandler *collision_handler;

	//int num_relate_changes;
	//relationship_t relate_change[MAX_MESSAGES];
	//relate_t relate_query[MAX_ENTITIES];
} zrc_t;

registry_t zrc_components(int count, ...);

#define ZRC_PAST_FRAME(zrc, n) (((zrc)->frame - (n)) & MASK_FRAMES)
#define ZRC_PREV_FRAME(zrc) ZRC_PAST_FRAME(zrc, 1)
#define ZRC_READ_FRAME(zrc) ZRC_PAST_FRAME(zrc, 0)
#define ZRC_WRITE_FRAME(zrc) ZRC_PAST_FRAME(zrc, -1)
#define ZRC_NEXT_FRAME(zrc) ZRC_PAST_FRAME(zrc, -2)

#define ZRC_HAD_PAST(zrc, name, id, n) (((zrc)->registry[ZRC_PAST_FRAME(zrc, n)][(id)&MASK_ENTITIES] & (1<<zrc_component_##name##)) == (1<<zrc_component_##name##))
#define ZRC_HAD(zrc, name, id) (((zrc)->registry[ZRC_PREV_FRAME(zrc)][(id)&MASK_ENTITIES] & (1<<zrc_component_##name##)) == (1<<zrc_component_##name##))
#define ZRC_HAS(zrc, name, id) (((zrc)->registry[ZRC_READ_FRAME(zrc)][(id)&MASK_ENTITIES] & (1<<zrc_component_##name##)) == (1<<zrc_component_##name##))

#define ZRC_GET_PAST(zrc, name, id, n) (&(zrc)->##name##[ZRC_PAST_FRAME(zrc, (n))][(id)&MASK_ENTITIES])
#define ZRC_GET_PREV(zrc, name, id) (&(zrc)->##name##[ZRC_PREV_FRAME(zrc)][(id)&MASK_ENTITIES])
#define ZRC_GET_READ(zrc, name, id) (&(zrc)->##name##[ZRC_READ_FRAME(zrc)][(id)&MASK_ENTITIES])
#define ZRC_GET_WRITE(zrc, name, id) (&(zrc)->##name##[ZRC_WRITE_FRAME(zrc)][(id)&MASK_ENTITIES])
#define ZRC_GET_NEXT(zrc, name, id) (&(zrc)->##name##[ZRC_NEXT_FRAME(zrc)][(id)&MASK_ENTITIES])
#define ZRC_GET(zrc, name, id) (ZRC_HAS(zrc, name, id) ? (const zrc_##name##_t *)ZRC_GET_READ(zrc, name, id) : 0)

#define ZRC_RECEIVE(zrc, name, id, start, var, code) \
	(var) = 0; \
	while ((*(start)) != ((zrc)->send_##name##[(id)&MASK_ENTITIES])) { \
		(var) = &(zrc)->##name##[(id)&MASK_ENTITIES][(*(start))&(max_##name##_messages-1)]; \
		code; \
		++(*(start)); \
	}

#define ZRC_PEEK(zrc, name, id, start) \
	((*(start)) != (zrc)->send_##name##[(id)&MASK_ENTITIES] ? &(zrc)->##name##[(id)&MASK_ENTITIES][(*(start))&(max_##name##_messages-1)] : 0)

#define ZRC_SEND(zrc, name, id, val) (zrc)->##name##[(id)&MASK_ENTITIES][((zrc)->send_##name##[(id)&MASK_ENTITIES]++)&(max_##name##_messages-1)] = *(val)

// components with no behavior (pure state)
#define ZRC_UPDATE0(zrc, name) do { \
		uint64_t start = stm_now(); \
		for (int i = 0; i < MAX_ENTITIES; ++i) { \
			int ignore_registry = zrc_component_##name## == zrc_component_registry; /*temp*/ \
			if (ignore_registry || ZRC_HAS(zrc, name, i)) { \
				##name##_t *prev = ZRC_GET_READ(zrc, name, i); \
				##name##_t *next = ZRC_GET_WRITE(zrc, name, i); \
				*next = *prev; \
			} \
		} \
		##name##_update(zrc); \
		uint64_t end = stm_now(); \
		(zrc)->times[zrc_component_##name##] = stm_diff(end, start); \
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
		(zrc)->times[zrc_component_##name##] = stm_diff(end, start); \
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
		(zrc)->times[zrc_component_##name##] = stm_diff(end, start); \
	} while (0)

#define ZRC_SPAWN(zrc, name, id, value) do { \
		id_t _id = (id); \
		zrc_assert(!ZRC_HAS(zrc, name, _id) && #name" already exists"); \
		*ZRC_GET_WRITE(zrc, registry, _id) |= ((1<<zrc_component_##name##) | (1<<zrc_component_registry)); \
		*ZRC_GET_WRITE(zrc, name, _id) = *(value); \
	} while (0)

#define ZRC_DESPAWN(zrc, name, id) (*ZRC_GET_WRITE(zrc, registry, id) &= ~(1<<zrc_component_##name##))
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
void zrc_update(zrc_t *);

void registry_startup(zrc_t *);
void registry_shutdown(zrc_t *);
void registry_update(zrc_t *);

void physics_startup(zrc_t *);
void physics_shutdown(zrc_t *);
void physics_create(zrc_t *, id_t, physics_t *);
void physics_delete(zrc_t *, id_t, physics_t *);
void physics_begin(zrc_t *, id_t, physics_t *);
void physics_update(zrc_t *);
void physics_end(zrc_t *, id_t, physics_t *);
id_t physics_query_ray(const zrc_t *, cpVect start, cpVect end, float radius);
id_t physics_query_point(const zrc_t *, cpVect point, float radius);

void visual_startup(zrc_t *);
void visual_shutdown(zrc_t *);
void visual_update(zrc_t *);

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

void locomotion_startup(zrc_t *);
void locomotion_shutdown(zrc_t *);
void locomotion_create(zrc_t *, id_t, locomotion_t *);
void locomotion_delete(zrc_t *, id_t, locomotion_t *);
void locomotion_update(zrc_t *, id_t, locomotion_t *);

void seek_startup(zrc_t *);
void seek_shutdown(zrc_t *);
void seek_create(zrc_t *, id_t, seek_t *);
void seek_delete(zrc_t *, id_t, seek_t *);
void seek_update(zrc_t *, id_t, seek_t *);

void align_startup(zrc_t *);
void align_shutdown(zrc_t *);
void align_create(zrc_t *, id_t, align_t *);
void align_delete(zrc_t *, id_t, align_t *);
void align_update(zrc_t *, id_t, align_t *);

void sense_startup(zrc_t *);
void sense_shutdown(zrc_t *);
void sense_create(zrc_t *, id_t, sense_t *);
void sense_delete(zrc_t *, id_t, sense_t *);
void sense_update(zrc_t *, id_t, sense_t *);

void relate_startup(zrc_t *);
void relate_shutdown(zrc_t *);
void relate_update(zrc_t *);
float relate_to_query(zrc_t *, id_t, id_t);
void team_update(zrc_t*);

void rl_startup(zrc_t *);
void rl_shutdown(zrc_t *);
void rl_create(zrc_t *, id_t, rl_t *);
void rl_delete(zrc_t *, id_t, rl_t *);
void rl_update(zrc_t *, id_t, rl_t *);


#ifdef __cplusplus
}
#endif

#endif