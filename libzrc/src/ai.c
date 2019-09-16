#include <zrc.h>
#include <zmath.h>
#include <string.h>

static float ability_vector[ABILITY_COUNT][ABILITY_COUNT];

static ability_id_t ability_match(const ai_t *ai, float *v);
static void ability_write(ability_id_t, float *);

void ai_startup(zrc_t *ai) {
	for (int i = 0; i < ABILITY_COUNT; ++i) {
		ability_vector[i][i] = 1;
	}
}
void ai_shutdown(zrc_t *ai) {

}
void ai_create(zrc_t *zrc, id_t id, ai_t *ai) {
}
void ai_delete(zrc_t *zrc, id_t id, ai_t *ai) {
}
void ai_update(zrc_t *zrc, id_t id, ai_t *ai) {
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		// gather state
		physics_t *physics = ZRC_GET(zrc, physics, i);
		caster_t *caster = ZRC_GET(zrc, caster, i);

		// calculate reward
		float next_reward = 0;
		damage_dealt_t *damage_dealt;
		ZRC_RECEIVE(zrc, damage_dealt, i, &ai->damage_dealt_index, damage_dealt, {
			next_reward += damage_dealt->health;
		});

		ai->reward = next_reward;
	}
}

void ai_observe(const zrc_t *zrc, id_t id, float *observation) {
	const ai_t *ai = ZRC_GET(zrc, ai, id);
	const physics_t *physics = ZRC_GET(zrc, physics, id);
	const sense_t *sense = ZRC_GET(zrc, sense, id);
	assert(ai && physics && sense);
	int op = 0;

	if (physics && sense) {
		for (int i = 0; i < SENSE_MAX_ENTITIES; ++i) {
			id_t sid = sense->entities[i];
			const physics_t *sphysics = ZRC_GET_READ(zrc, physics, sid);
			const caster_t *scaster = ZRC_GET_READ(zrc, caster, sid);
			const life_t *slife = ZRC_GET_READ(zrc, life, sid);

			float sradius = sphysics->radius / MAP_SCALE;
			observation[op++] = sradius;
			cpVect soffset = cpvsub(sphysics->position, physics->position);
			cpVect sscale_offset = cpvmult(soffset, 1 / sense->range);
			observation[op++] = sscale_offset.x;
			observation[op++] = sscale_offset.y;
			cpVect srel_scale_offset = cpvrotate(sscale_offset, cpvforangle(physics->angle));
			observation[op++] = srel_scale_offset.x;
			observation[op++] = srel_scale_offset.y;
			float angle = sphysics->angle;
			observation[op++] = angle;
			float srel_angle = angle - physics->angle;
			observation[op++] = srel_angle;
			cpVect svelocity = sphysics->velocity;
			cpVect sscale_velocity = cpvmult(svelocity, 1 / sense->range);
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
				float scaled_uptime = scaster_ability->uptime / max(1, sability->channel);
				observation[op++] = scaled_uptime;
				float scaled_downtime = scaster_ability->downtime / max(1, sability->cooldown);
				observation[op++] = scaled_downtime;
				cpVect starget = cpv(scaster_ability->target.point[0], scaster_ability->target.point[1]);
				cpVect starget_offset = cpvsub(starget, physics->position);
				cpVect starget_scale_offset = cpvmult(starget_offset, 1 / sense->range);
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

			float healthpct = slife->health / slife->max_health;
			observation[op++] = healthpct;
			float manapct = slife->mana / slife->max_mana;
			observation[op++] = manapct;
			float ragepct = slife->rage / slife->max_rage;
			observation[op++] = ragepct;
		}
	}
	assert(op == AI_OBSERVATION_LENGTH);
	// todo read all sensed entities, sort?, observe ai->state, write all to numpy
}

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
				// todo unit target
				ZRC_SEND(zrc, cast, id, &cast);
			}
		}
	}
	assert(ap == AI_ACTION_LENGTH);
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
	return maxi;
}
