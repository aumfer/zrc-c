#include <zrc.h>
#include <zmath.h>
#include <string.h>
#include <stdio.h>

static float ability_vector[ABILITY_COUNT][ABILITY_COUNT];

static ability_id_t ability_match(const ai_t *ai, float *v);

#define isvalid(v) (!v || isnormal(v))

void ai_startup(zrc_t *zrc) {
	//printf("ai %zu\n", sizeof(zrc->ai));
	for (int i = 0; i < ABILITY_COUNT; ++i) {
		ability_vector[i][i] = 1;
	}
}
void ai_shutdown(zrc_t *zrc) {

}
void ai_create(zrc_t *zrc, id_t id, ai_t *ai) {
}
void ai_delete(zrc_t *zrc, id_t id, ai_t *ai) {
}
void ai_update(zrc_t *zrc, id_t id, ai_t *ai) {
	float next_reward = 0;

	// give baseline reward [0,1] for being near entities
	physics_t *physics = ZRC_GET(zrc, physics, id);
	sense_t *sense = ZRC_GET(zrc, sense, id);
	if (physics && sense) {
		float sum_dist = 0;
		int num_dist = 0;
		for (int i = 0; i < sense->num_entities; ++i) {
			id_t sid = sense->entities[i];
			physics_t *sphysics = ZRC_GET(zrc, physics, sid);
			if (physics) {
				float d = cpvdistsq(physics->position, sphysics->position);
				sum_dist += d;
				++num_dist;
			}
		}
		if (num_dist) {
			sum_dist /= num_dist;
			next_reward += 1 / max(1, sum_dist);
		}
	}

	// reward for damage dealt
	damage_dealt_t *damage_dealt;
	ZRC_RECEIVE(zrc, damage_dealt, id, &ai->damage_dealt_index, damage_dealt, {
		next_reward += damage_dealt->health;
	});

	id_t *killed;
	ZRC_RECEIVE(zrc, got_kill, id, &ai->got_kill_index, killed, {
		next_reward += 100;
	});

	next_reward /= 1000;
	ai->reward = next_reward;

	ai->total_reward += ai->reward;
}
void ai_observe(const zrc_t *zrc, id_t id, float *observation) {
	const ai_t *ai = ZRC_GET(zrc, ai, id);
	const physics_t *physics = ZRC_GET(zrc, physics, id);
	const sense_t *sense = ZRC_GET(zrc, sense, id);
	assert(ai && physics && sense);
	int op = 0;

	for (int i = 0; i < SENSE_MAX_ENTITIES; ++i) {
		id_t sid = sense->entities[i];
		const physics_t *sphysics = ZRC_GET_READ(zrc, physics, sid);
		const caster_t *scaster = ZRC_GET_READ(zrc, caster, sid);
		const life_t *slife = ZRC_GET_READ(zrc, life, sid);

		float isme = id == sid ? 1.0f : 0.0f;
		observation[op++] = isme;
		float sradius = sphysics->radius / MAP_SCALE;
		observation[op++] = sradius;
		cpVect soffset = cpvsub(sphysics->position, physics->position);
		cpVect sscale_offset = cpvmult(soffset, 1 / sense->range);
		observation[op++] = sscale_offset.x;
		observation[op++] = sscale_offset.y;
		cpVect srel_scale_offset = cpvrotate(sscale_offset, cpvforangle(physics->angle));
		observation[op++] = srel_scale_offset.x;
		observation[op++] = srel_scale_offset.y;
		float scaled_dist = cpvdistsq(physics->position, sphysics->position) / max(1, sense->range*sense->range);
		observation[op++] = scaled_dist;
		float angle = sphysics->angle;
		observation[op++] = angle / (CP_PI * 2);
		float srel_angle = angle - physics->angle;
		observation[op++] = srel_angle / (CP_PI * 2);
		cpVect svelocity = sphysics->velocity;
		cpVect sscale_velocity = cpvmult(svelocity, 1 / max(1, sense->range));
		observation[op++] = sscale_velocity.x;
		observation[op++] = sscale_velocity.y;
		cpVect srel_scale_velocity = cpvrotate(sscale_velocity, cpvforangle(physics->angle));
		observation[op++] = srel_scale_velocity.x;
		observation[op++] = srel_scale_velocity.y;

		for (int j = 0; j < CASTER_MAX_ABLITIES; ++j) {
			const caster_ability_t *scaster_ability = &scaster->abilities[j];
			const ability_t *sability = &zrc->ability[scaster_ability->ability];
			memcpy(&observation[op], ability_vector[scaster_ability->ability], sizeof(ability_vector[scaster_ability->ability]));
			op += ABILITY_COUNT;
			float scaled_range = sability->range / max(1, sense->range);
			observation[op++] = scaled_range;
			float scaled_uptime = scaster_ability->uptime / max(1, sability->channel);
			observation[op++] = scaled_uptime;
			float scaled_downtime = scaster_ability->downtime / max(1, sability->cooldown);
			observation[op++] = scaled_downtime;
			cpVect starget = cpv(scaster_ability->target.point[0], scaster_ability->target.point[1]);
			cpVect starget_offset = cpvsub(starget, physics->position);
			cpVect starget_scale_offset = cpvmult(starget_offset, 1 / max(1, sense->range));
			observation[op++] = starget_scale_offset.x;
			observation[op++] = starget_scale_offset.y;
			cpVect starget_rel_scale_offset = cpvrotate(starget_scale_offset, cpvforangle(physics->angle));
			observation[op++] = starget_rel_scale_offset.x;
			observation[op++] = starget_rel_scale_offset.y;
			cpVect starget_srel_scale_offset = cpvrotate(starget_scale_offset, cpvforangle(sphysics->angle));
			observation[op++] = starget_srel_scale_offset.x;
			observation[op++] = starget_srel_scale_offset.y;
			float want_cast = (scaster_ability->cast_flags & CAST_WANTCAST) == CAST_WANTCAST ? 1.0f : 0.0f;
			observation[op++] = want_cast;
			float is_cast = (scaster_ability->cast_flags & CAST_ISCAST) == CAST_ISCAST ? 1.0f : 0.0f;
			observation[op++] = is_cast;
		}

		float healthpct = slife->health / max(1, slife->max_health);
		observation[op++] = healthpct;
		float manapct = slife->mana / max(1, slife->max_mana);
		observation[op++] = manapct;
		float ragepct = slife->rage / max(1, slife->max_rage);
		observation[op++] = ragepct;
	}
	assert(op == AI_OBSERVATION_LENGTH);
}

static unsigned has_cast[ABILITY_COUNT];

void ai_act(zrc_t *zrc, id_t id, float *action) {
	ai_t *ai = ZRC_GET(zrc, ai, id);
	physics_t *physics = ZRC_GET(zrc, physics, id);
	assert(ai && physics);
	if (!physics || !ai) return;

	int ap = 0;
	flight_thrust_t flight_thrust = {
		.thrust = { action[ap++], action[ap++] },
		.turn = action[ap++]
	};
	ZRC_SEND(zrc, flight_thrust, id, &flight_thrust);
	ability_id_t ability_id = ability_match(ai, &action[ap]);
	ap += ABILITY_COUNT;
	if (ability_id > ABILITY_NONE) {
		ability_t *ability = &zrc->ability[ability_id];
		caster_t *caster = ZRC_GET(zrc, caster, id);
		if (caster) {
			caster_ability_id_t caster_ability_id = CASTER_ABILITY_INVALID;
			for (int i = 0; i < CASTER_MAX_ABLITIES; ++i) {
				if (caster->abilities[i].ability == ability_id) {
					caster_ability_id = i;
					break;
				}
			}
			if (caster_ability_id != CASTER_ABILITY_INVALID) {
				cpVect target_rel_scale_offset = cpv(action[ap++], action[ap++]);
				cpVect target_scale_offset = cpvrotate(target_rel_scale_offset, cpvforangle(physics->angle));
				cpVect target_offset = cpvmult(target_scale_offset, ability->range);
				cpVect target_point = cpvadd(physics->position, target_offset);
				cast_t cast = {
					.caster_ability = caster_ability_id,
					.cast_flags = CAST_WANTCAST,
					.target.point[0] = target_point.x,
					.target.point[1] = target_point.y
				};
				if (!has_cast[ability_id]) {
					printf("casting %d to %.2f %.2f\n", ability_id, target_point.x, target_point.y);
				}
				++has_cast[ability_id];
				// todo unit target
				ZRC_SEND(zrc, cast, id, &cast);
			}
		}
	}
	assert(ap <= AI_ACTION_LENGTH);
}


static ability_id_t ability_match(const ai_t *ai, float *v) {
	float dots[ABILITY_COUNT];
	for (int i = 0; i < ABILITY_COUNT; ++i) {
		int n = ABILITY_COUNT;
		int incx = 1;
		int incy = 1;
		float d = sdot(&n, v, &incx, ability_vector[i], &incy);
		dots[i] = d;
	}
	int maxi = -1;
	float maxv;
	for (int i = 0; i < ABILITY_COUNT; ++i) {
		if (!i || dots[i] > maxv) {
			maxi = i;
			maxv = dots[i];
		}
	}
	//printf("best ability %i %.4f\n", maxi, maxv);
	return maxi;
}
