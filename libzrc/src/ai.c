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
void ai_updatex(zrc_t *zrc, id_t id, ai_t *ai) {
	float next_reward = 0;

	physics_t *physics = ZRC_GET(zrc, physics, id);
	sense_t *sense = ZRC_GET(zrc, sense, id);
	life_t *life = ZRC_GET(zrc, life, id);
	team_t *team = ZRC_GET(zrc, team, id);

#if 0
	// give baseline reward [0,1] for being near entities
	if (physics && sense) {
		// base-baseline reward toward (0,0) in case we can't see anyone
		//next_reward += (1 / cpvlengthsq(physics->position));
		float sum_dist = 0;
		int num_dist = 0;
		for (int i = 0; i < sense->num_entities; ++i) {
			id_t sid = sense->entities[i];
			physics_t *sphysics = ZRC_GET(zrc, physics, sid);
			if (physics) {
				float d = cpvdistsq(physics->position, sphysics->position);
				sum_dist += 1 / max(1, d);
				++num_dist;
			}
		}
		next_reward += sum_dist * TICK_RATE;
	}
#endif

	// reward for damage dealt
	damage_dealt_t *damage_dealt;
	ZRC_RECEIVE(zrc, damage_dealt, id, &ai->damage_dealt_index, damage_dealt, {
		team_t *tteam = ZRC_GET(zrc, team, damage_dealt->to);
		if (team && tteam) {
			if (*tteam != *team) {
				next_reward += damage_dealt->health;
			} else {
				next_reward -= damage_dealt->health;
			}
		}
	});

	id_t *killed;
	ZRC_RECEIVE(zrc, got_kill, id, &ai->got_kill_index, killed, {
		team_t *tteam = ZRC_GET(zrc, team, *killed);
		if (team && tteam) {
			printf("%d killed %d\n", *team, *tteam);
			if (*tteam != *team) {
				next_reward += 1000;
			} else {
				next_reward -= 1000;
			}
		}
	});

	if (life) {
		damage_t *damage;
		ZRC_RECEIVE(zrc, damage, id, &ai->damage_taken_index, damage, {
			next_reward -= damage->health * (1 / max(0.1f, life->health / life->max_health));
		});
	}

	next_reward /= 1000;
	ai->reward = next_reward;

	ai->total_reward += ai->reward;
}

void ai_update(zrc_t *zrc, id_t id, ai_t *ai) {
	if (!ZRC_HAS(zrc, physics, id) || !ZRC_HAD(zrc, physics, id)) return;

	ai->reward = 0;

	physics_t *physics = ZRC_GET_READ(zrc, physics, id);
	physics_t *pphysics = ZRC_GET_PREV(zrc, physics, id);
	
	cpVect front = cpvforangle(physics->angle);
	cpVect pfront = cpvforangle(pphysics->angle);
	cpVect goal_front = cpvforangle(ai->goala);

	float ad = cpvdot(front, goal_front);
	float pad = cpvdot(pfront, goal_front);
	float ar = (ad - pad);
	ai->reward += ar / physics->max_spin;
	float pd = cpvdistsq(physics->position, ai->goalp);
	float ppd = cpvdistsq(pphysics->position, ai->goalp);
	float pr = (ppd - pd);
	ai->reward += pr / physics->max_speed;
	//ai->reward += 1 / max(1, pd);
	if (pd < (physics->radius*physics->radius) && ad > 0.8f) {
		ai->reward += 10;
		ai->done = 1;
	}

	ai->total_reward += ai->reward;
}

void ai_observe(const zrc_t *zrc, id_t id, unsigned frame, float *observation) {
	const ai_t *ai = ZRC_GET_PAST(zrc, ai, id, frame);
	const physics_t *physics = ZRC_GET_PAST(zrc, physics, id, frame);

	cpVect front = cpvforangle(physics->angle);
	cpVect goal_front = cpvforangle(ai->goala);
	float goald = cpvdistsq(physics->position, ai->goalp);
#define LIDAR 16

	int op = 0;
	float distances[LIDAR];
	for (int i = 0; i < LIDAR; ++i) {
		float a = ((float)i / LIDAR) * (CP_PI * 2);
		cpVect v = cpvforangle(a);
		v = cpvrotate(v, front);
		float oa = cpvdot(v, goal_front);
		observation[op++] = oa;

		cpVect p = cpvadd(physics->position, v);
		float d = cpvdistsq(p, ai->goalp);
		distances[i] = goald - d;
	}

	float min_distance = FLT_MAX;
	float max_distance = -FLT_MAX;
	for (int i = 0; i < LIDAR; ++i) {
		min_distance = min(min_distance, distances[i]);
		max_distance = max(max_distance, distances[i]);
	}
	for (int i = 0; i < LIDAR; ++i) {
		double d = (distances[i] - min_distance) / (max_distance - min_distance);
		observation[op++] = (float)d;
	}

	assert(op == AI_OBSERVATION_LENGTH);
	for (int i = 0; i < op; ++i) {
		assert(observation[i] >= -1-FLT_EPSILON && observation[i] <= 1+FLT_EPSILON);
	}
}

void ai_observex(const zrc_t *zrc, id_t id, unsigned frame, float *observation) {
	const ai_t *ai = ZRC_GET_PAST(zrc, ai, id, frame);
	const physics_t *physics = ZRC_GET_PAST(zrc, physics, id, frame);
	const sense_t *sense = ZRC_GET_PAST(zrc, sense, id, frame);
	const team_t *team = ZRC_GET_PAST(zrc, team, id, frame);
	zrc_assert(ai && physics && sense);
	int op = 0;

	cpVect front = cpvforangle(physics->angle);

	for (int i = 0; i < SENSE_MAX_ENTITIES; ++i) {
		id_t sid = sense->entities[i];
		const physics_t *sphysics = ZRC_GET_PAST(zrc, physics, sid, frame);
		const caster_t *scaster = ZRC_GET_PAST(zrc, caster, sid, frame);
		const life_t *slife = ZRC_GET_PAST(zrc, life, sid, frame);
		const team_t *steam = ZRC_GET_PAST(zrc, team, sid, frame);

		//if (sid == id) continue;

		float isme = id == sid ? 1.0f : 0.0f;
		observation[op++] = isme;
		float myteam = *team == *steam ? 1.0f : 0.0f;
		observation[op++] = myteam;
		if (ZRC_HAD_PAST(zrc, physics, sid, frame)) {
			cpVect sfront = cpvforangle(sphysics->angle);

			float sradius = sphysics->radius / MAP_SCALE;
			observation[op++] = sradius;

			//observation[op++] = sphysics->position.x / MAP_SCALE;
			//observation[op++] = sphysics->position.y / MAP_SCALE;
			//observation[op++] = 0;
			//observation[op++] = fmodf(sphysics->angle, (CP_PI * 2));
			//observation[op++] = sphysics->velocity.x / sphysics->max_speed;
			//observation[op++] = sphysics->velocity.y / sphysics->max_speed;
			//observation[op++] = 0;
			//observation[op++] = 0;

			cpVect soffset = cpvsub(sphysics->position, physics->position);
			cpVect sscale_offset = cpvmult(soffset, 1 / sense->range);
			cpVect srel_scale_offset = cpvrotate(sscale_offset, front);
			observation[op++] = srel_scale_offset.x;
			observation[op++] = srel_scale_offset.y;
			float scaled_dist = cpvdistsq(physics->position, sphysics->position) / max(1, sense->range*sense->range);
			observation[op++] = scaled_dist;
			float angle = sphysics->angle;
			float srel_angle = angle - physics->angle;
			float srel_scale_angle = fmodf(srel_angle, (CP_PI * 2));
			observation[op++] = srel_scale_angle;
			cpVect svelocity = sphysics->velocity;
			cpVect sscale_velocity = cpvmult(svelocity, 1 / max(1, sense->range));
			cpVect srel_scale_velocity = cpvrotate(sscale_velocity, front);
			observation[op++] = srel_scale_velocity.x;
			observation[op++] = srel_scale_velocity.y;
			cpVect srels_scale_velocity = cpvrotate(sscale_velocity, sfront);
			observation[op++] = srels_scale_velocity.x;
			observation[op++] = srels_scale_velocity.y;
		} else {
			memset(&observation[op], 0, sizeof(float) * 9);
			op += 9;
		}

#if 0
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
			// temp remove NaN
			starget.x = isvalid(starget.x) ? starget.x : sphysics->position.x;
			starget.y = isvalid(starget.y) ? starget.y : sphysics->position.y;
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
#endif
#if 0
		if (ZRC_HAD_PAST(zrc, life, sid, frame)) {
			float healthpct = slife->health / max(1, slife->max_health);
			observation[op++] = healthpct;
			float manapct = slife->mana / max(1, slife->max_mana);
			observation[op++] = manapct;
			float ragepct = slife->rage / max(1, slife->max_rage);
			observation[op++] = ragepct;
		} else {
			memset(&observation[op], 0, sizeof(float) * 3);
			op += 3;
		}
#endif
	}
	zrc_assert(op == AI_OBSERVATION_LENGTH);
#if 0
	for (int i = 0; i < AI_OBSERVATION_LENGTH; ++i) {
		if (!isvalid(observation[i])) {
			observation[i] = 0;
			zrc_assert(0);
		}
		zrc_assert(isvalid(observation[i]));
	}
#endif
}

static unsigned has_cast[ABILITY_COUNT];

static float step(float v) {
	return v < 0 ? -1.0f : (v > 0 ? +1.0f : 0.0f);
}

void ai_act(zrc_t *zrc, id_t id, float *action) {
	for (int i = 0; i < AI_ACTION_LENGTH; ++i) {
		if (!isvalid(action[i])) {
			action[i] = 0;
			//zrc_assert(0);
		}
		zrc_assert(isvalid(action[i]));
	}
	ai_t *ai = ZRC_GET(zrc, ai, id);
	physics_t *physics = ZRC_GET(zrc, physics, id);
	if (!physics || !ai) return;

	int ap = 0;
	flight_thrust_t flight_thrust = {
		.thrust = { action[ap++], action[ap++] },
		.turn = action[ap++]
	};
	flight_thrust.thrust[0] = step(flight_thrust.thrust[0]);
	flight_thrust.thrust[1] = step(flight_thrust.thrust[1]);
	flight_thrust.turn = step(flight_thrust.turn);
	ZRC_SEND(zrc, flight_thrust, id, &flight_thrust);
#if 0
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
				cpVect target_rel_offset = cpvmult(target_rel_scale_offset, ability->range);
				cpVect target_offset = cpvrotate(target_rel_offset, cpvforangle(physics->angle));
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
#endif
	zrc_assert(ap <= AI_ACTION_LENGTH);
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
