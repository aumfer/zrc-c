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
#undef NDEBUG
#include <assert.h>

typedef uint16_t id_t;
#define ID_INVALID ((id_t)-1)

#define MAX_ENTITIES (16384/2)
#define MASK_ENTITIES (MAX_ENTITIES-1)
#define MAX_FRAMES (64/2)
#define MASK_FRAMES (MAX_FRAMES-1)

// todo use per-type message counts to save space
#define MAX_MESSAGES (64/2)
#define MASK_MESSAGES (MAX_MESSAGES-1)

#define TICK_RATE (1.0f/60.0f)
#define MAP_SCALE 16

#define randf() ((float)rand() / RAND_MAX)

#define zrc_assert assert

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
	zrc_locomotion,
	zrc_seek,
	zrc_sense,
	//zrc_relate,
	zrc_team,
	zrc_ai,
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

	cpBody *body;
	cpShape *shape;

	unsigned force_index;
} physics_t;

typedef struct physics_force {
	cpVect force;
	float torque;
} physics_force_t;

typedef struct physics_controller {
	cpBody *body;
	cpConstraint *pivot;
	cpConstraint *gear;

	unsigned velocity_index;
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

typedef struct flight_thrust {
	cpVect thrust;
	float turn;
} flight_thrust_t;

typedef struct flight {
	float max_thrust;
	float max_turn;

	float thrust_control_rate;
	float turn_control_rate;

	cpVect thrust;
	float turn;

	unsigned thrust_index;
} flight_t;

typedef struct life {
	float health, max_health, strength, constitution;
	float mana, max_mana, focus, willpower;
	float rage, max_rage, serenity, temper;

	unsigned damage_index;
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

typedef struct damage {
	id_t from;
	ability_id_t ability;
	float health;
} damage_t;

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

typedef struct cast {
	caster_ability_id_t caster_ability;
	ability_target_t target;
	cast_flags_t cast_flags;
} cast_t;

#define CASTER_MAX_ABLITIES ABILITY_COUNT
typedef struct caster {
	caster_ability_t abilities[CASTER_MAX_ABLITIES];

	unsigned cast_index;
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

typedef double(*locomotion_behavior_t)(const zrc_t *, id_t, cpVect point);

typedef struct locomotion {
	locomotion_behavior_t behaviors[1]; // temp
	int num_behaviors;

	unsigned locomotion_behavior_index;
} locomotion_t;

typedef struct seek_to {
	cpVect point;
} seek_to_t;

typedef struct seek {
	cpVect point;
	unsigned seek_index;
} seek_t;

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

#define AI_LIDAR 16

#define AI_LOCOMOTION_ACT_LENGTH 3 // thrust.x, thrust.y, turn
#define AI_LOCOMOTION_ENTITY_LENGTH 2 // thrust, turn
#define AI_LOCOMOTION_OBS_LENGTH ((AI_LIDAR*AI_LOCOMOTION_ENTITY_LENGTH)+AI_LOCOMOTION_ACT_LENGTH)
#define AI_SENSE_ENTITY_LENGTH 2 // distance, team
#define AI_SENSE_OBS_LENGTH (AI_LIDAR*AI_SENSE_ENTITY_LENGTH)
#define AI_SENSE_ACT_LENGTH AI_LOCOMOTION_OBS_LENGTH // + AI_ABILITY_OBS_LENGTH

typedef enum ai_train_flags {
	AI_TRAIN_NONE = 0,
	AI_TRAIN_LOCOMOTION = 1,
	AI_TRAIN_SENSE = 2
} ai_train_flags_t;

typedef struct ai {
	int done;
	ai_train_flags_t train_flags;
	float total_reward;
	float reward;

	struct {
		cpVect goalp;
		float goala;
	} train_locomotion;

	float sense_obs[AI_SENSE_OBS_LENGTH];
	float sense_act[AI_SENSE_ACT_LENGTH];
	float locomotion_obs[AI_LOCOMOTION_OBS_LENGTH];
	float locomotion_act[AI_LOCOMOTION_ACT_LENGTH];

	unsigned damage_dealt_index;
	unsigned got_kill_index;
	unsigned damage_taken_index;
} ai_t;

typedef uint32_t team_t;

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
	sense_t sense[MAX_FRAMES][MAX_ENTITIES];
	//relate_t relate[MAX_FRAMES][MAX_ENTITIES];
	ai_t ai[MAX_FRAMES][MAX_ENTITIES];
	team_t team[MAX_FRAMES][MAX_ENTITIES];

	// transient // todo volatile?
	unsigned num_damage[MAX_ENTITIES];
	damage_t damage[MAX_ENTITIES][MAX_MESSAGES];
	unsigned num_physics_force[MAX_ENTITIES];
	physics_force_t physics_force[MAX_ENTITIES][MAX_MESSAGES];
	unsigned num_physics_controller_velocity[MAX_ENTITIES];
	physics_controller_velocity_t physics_controller_velocity[MAX_ENTITIES][MAX_MESSAGES];
	unsigned num_cast[MAX_ENTITIES];
	cast_t cast[MAX_ENTITIES][MAX_MESSAGES];
	unsigned num_locomotion_behavior[MAX_ENTITIES];
	locomotion_behavior_t locomotion_behavior[MAX_ENTITIES][MAX_MESSAGES];
	unsigned num_flight_thrust[MAX_ENTITIES];
	flight_thrust_t flight_thrust[MAX_ENTITIES][MAX_MESSAGES];
	unsigned num_seek_to[MAX_ENTITIES];
	seek_to_t seek_to[MAX_ENTITIES][MAX_MESSAGES];
	unsigned num_damage_dealt[MAX_ENTITIES];
	damage_dealt_t damage_dealt[MAX_ENTITIES][MAX_MESSAGES];
	unsigned num_got_kill[MAX_ENTITIES];
	id_t got_kill[MAX_ENTITIES][MAX_MESSAGES];

	unsigned frame;
	uint64_t times[zrc_component_count];
	timer_t timer;
	double accumulator;
	moving_average_t tick_fps;
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

#define ZRC_HAD_PAST(zrc, name, id, n) (((zrc)->registry[ZRC_PAST_FRAME(zrc, n)][(id)&MASK_ENTITIES] & (1<<zrc_##name##)) == (1<<zrc_##name##))
#define ZRC_HAD(zrc, name, id) (((zrc)->registry[ZRC_PREV_FRAME(zrc)][(id)&MASK_ENTITIES] & (1<<zrc_##name##)) == (1<<zrc_##name##))
#define ZRC_HAS(zrc, name, id) (((zrc)->registry[ZRC_READ_FRAME(zrc)][(id)&MASK_ENTITIES] & (1<<zrc_##name##)) == (1<<zrc_##name##))

#define ZRC_GET_PAST(zrc, name, id, n) (&(zrc)->##name##[ZRC_PAST_FRAME(zrc, (n))][(id)&MASK_ENTITIES])
#define ZRC_GET_PREV(zrc, name, id) (&(zrc)->##name##[ZRC_PREV_FRAME(zrc)][(id)&MASK_ENTITIES])
#define ZRC_GET_READ(zrc, name, id) (&(zrc)->##name##[ZRC_READ_FRAME(zrc)][(id)&MASK_ENTITIES])
#define ZRC_GET_WRITE(zrc, name, id) (&(zrc)->##name##[ZRC_WRITE_FRAME(zrc)][(id)&MASK_ENTITIES])
#define ZRC_GET_NEXT(zrc, name, id) (&(zrc)->##name##[ZRC_NEXT_FRAME(zrc)][(id)&MASK_ENTITIES])
#define ZRC_GET(zrc, name, id) (ZRC_HAS(zrc, name, id) ? ZRC_GET_READ(zrc, name, id) : 0)

#define ZRC_RECEIVE(zrc, name, id, start, var, code) \
	for (unsigned i = (*(start)); i < (zrc)->num_##name##[(id)&MASK_ENTITIES]; ++i) { \
		(var) = &(zrc)->##name##[(id)&MASK_ENTITIES][i&MASK_MESSAGES]; \
		code; \
		++(*(start)); \
	}

#define ZRC_SEND(zrc, name, id, val) (zrc)->##name##[(id)&MASK_ENTITIES][((zrc)->num_##name##[(id)&MASK_ENTITIES]++)&MASK_MESSAGES] = *(val)

// components with no behavior (pure state)
#define ZRC_UPDATE0(zrc, name) do { \
		uint64_t start = stm_now(); \
		for (int i = 0; i < MAX_ENTITIES; ++i) { \
			int ignore_registry = zrc_##name## == zrc_registry; /*temp*/ \
			if (ignore_registry || ZRC_HAS(zrc, name, i)) { \
				##name##_t *prev = ZRC_GET_READ(zrc, name, i); \
				##name##_t *next = ZRC_GET_WRITE(zrc, name, i); \
				*next = *prev; \
			} \
		} \
		##name##_update(zrc); \
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
		id_t _id = (id); \
		zrc_assert(!ZRC_HAS(zrc, name, _id)); \
		*ZRC_GET_WRITE(zrc, registry, _id) |= ((1<<zrc_##name##) | (1<<zrc_registry)); \
		*ZRC_GET_WRITE(zrc, name, _id) = *(value); \
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

void ai_startup(zrc_t *);
void ai_shutdown(zrc_t *);
void ai_create(zrc_t *, id_t, ai_t *);
void ai_delete(zrc_t *, id_t, ai_t *);
void ai_update(zrc_t *, id_t, ai_t *);
void ai_observe_locomotion(zrc_t *, id_t id, float *numpyarray);
void ai_observe_locomotion_train(zrc_t *, id_t id, float *numpyarray);
void ai_act_locomotion(zrc_t *, id_t id, float *numpyarray);
void ai_observe_sense(zrc_t *zrc, id_t id, float *observation);
void ai_act_sense(zrc_t *zrc, id_t id, float *action);


#ifdef __cplusplus
}
#endif

#endif