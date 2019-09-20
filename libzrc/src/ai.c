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
void ai_update_train_sense(zrc_t *zrc, id_t id, ai_t *ai) {
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
		puts("recv dmg");
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
	ai->train_sense.reward = next_reward;

	ai->train_sense.total_reward += ai->train_sense.reward;
}

void ai_update_train_locomotion(zrc_t *zrc, id_t id, ai_t *ai) {
	if (!ZRC_HAS(zrc, physics, id) || !ZRC_HAD(zrc, physics, id)) return;

	float reward = 0;

	physics_t *physics = ZRC_GET_READ(zrc, physics, id);
	physics_t *pphysics = ZRC_GET_PREV(zrc, physics, id);

	cpVect goalp = ai->train_locomotion.goalp;
	float goala = ai->train_locomotion.goala;
	
	cpVect front = cpvforangle(physics->angle);
	cpVect pfront = cpvforangle(pphysics->angle);
	cpVect goal_front = cpvforangle(goala);

	float ad = cpvdot(front, goal_front);
	float pad = cpvdot(pfront, goal_front);
	float ar = (ad - pad);
	reward += ar / physics->max_spin;
	//ai->reward = ad;
	float pd = cpvdistsq(physics->position, goalp);
	float ppd = cpvdistsq(pphysics->position, goalp);
	float pr = (ppd - pd);
	reward += pr / physics->max_speed;
	//ai->reward += 1 / max(1, pd);
	if (pd < physics->radius && ad > 0.8f) {
		reward += 10;
		ai->done = 1;
	}

	ai->train_locomotion.reward = reward;
	ai->train_locomotion.total_reward += reward;
}

void ai_update(zrc_t *zrc, id_t id, ai_t *ai) {
	if ((ai->train_flags & AI_TRAIN_LOCOMOTION) == AI_TRAIN_LOCOMOTION) {
		ai_update_train_locomotion(zrc, id, ai);
	}
	if ((ai->train_flags & AI_TRAIN_SENSE) == AI_TRAIN_SENSE) {
		ai_update_train_sense(zrc, id, ai);
	}
}

void ai_observe_locomotion(const zrc_t *zrc, id_t id, unsigned frame, float *observation) {
	const ai_t *ai = ZRC_GET_PAST(zrc, ai, id, frame);
	if (!ai) return;

	memcpy(observation, ai->locomotion_obs, AI_LOCOMOTION_OBS_LENGTH * sizeof(float));
}

void ai_observe_locomotion_train(const zrc_t *zrc, id_t id, unsigned frame, float *observation) {
	const ai_t *ai = ZRC_GET_PAST(zrc, ai, id, frame);
	const physics_t *physics = ZRC_GET_PAST(zrc, physics, id, frame);

	cpVect goalp = ai->train_locomotion.goalp;
	float goala = ai->train_locomotion.goala;

	cpVect front = cpvforangle(physics->angle);
	cpVect goal_front = cpvforangle(goala);
	float goald = cpvdistsq(physics->position, goalp);

	int op = 0;
	float distances[AI_LIDAR];
	for (int i = 0; i < AI_LIDAR; ++i) {
		float a = ((float)i / AI_LIDAR) * (CP_PI * 2);
		cpVect v = cpvforangle(a);
		v = cpvrotate(v, front);
		float oa = cpvdot(v, goal_front);
		observation[op++] = oa;

		cpVect p = cpvadd(physics->position, v);
		float d = cpvdistsq(p, goalp);
		distances[i] = goald - d;
	}

	float min_distance = FLT_MAX;
	float max_distance = -FLT_MAX;
	for (int i = 0; i < AI_LIDAR; ++i) {
		min_distance = min(min_distance, distances[i]);
		max_distance = max(max_distance, distances[i]);
	}
	for (int i = 0; i < AI_LIDAR; ++i) {
		double d = (distances[i] - min_distance) / (max_distance - min_distance);
		observation[op++] = (float)d;
	}

	assert(op == AI_LOCOMOTION_OBS_LENGTH);
	for (int i = 0; i < op; ++i) {
		assert(observation[i] >= -1-FLT_EPSILON && observation[i] <= 1+FLT_EPSILON);
	}
}

void ai_observe_sense(const zrc_t *zrc, id_t id, id_t sid, unsigned frame, float *observation) {
	const ai_t *ai = ZRC_GET_PAST(zrc, ai, id, frame);
	const physics_t *physics = ZRC_GET_PAST(zrc, physics, id, frame);
	const sense_t *sense = ZRC_GET_PAST(zrc, sense, id, frame);
	const team_t *team = ZRC_GET_PAST(zrc, team, id, frame);
	zrc_assert(ai && physics && sense);
	int op = 0;

	memcpy(observation, ai->locomotion_obs, sizeof(ai->locomotion_obs));
	op += AI_LOCOMOTION_OBS_LENGTH;

	cpVect front = cpvforangle(physics->angle);
	float range = max(1, sense->range);

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
		cpVect sscale_offset = cpvmult(soffset, 1 / range);
		cpVect srel_scale_offset = cpvrotate(sscale_offset, front);
		observation[op++] = srel_scale_offset.x;
		observation[op++] = srel_scale_offset.y;
		float scaled_dist = cpvdistsq(physics->position, sphysics->position) / range*range;
		observation[op++] = scaled_dist;
		float angle = sphysics->angle;
		float srel_angle = angle - physics->angle;
		float srel_scale_angle = fmodf(srel_angle, (CP_PI * 2));
		//float saligned = cpvdot(front, sfront);
		observation[op++] = srel_scale_angle;
		cpVect svelocity = sphysics->velocity;
		cpVect sscale_velocity = cpvmult(svelocity, 1 / sphysics->max_speed);
		cpVect srel_scale_velocity = cpvrotate(sscale_velocity, front);
		observation[op++] = srel_scale_velocity.x;
		observation[op++] = srel_scale_velocity.y;
		cpVect srels_scale_velocity = cpvrotate(sscale_velocity, sfront);
		observation[op++] = srels_scale_velocity.x;
		observation[op++] = srels_scale_velocity.y;
	}
	else {
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

	zrc_assert(op == AI_SENSE_OBS_LENGTH);
#if 0
	for (int i = 0; i < AI_SENSE_OBS_LENGTH; ++i) {
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

void ai_act_sensex(zrc_t *zrc, id_t id, id_t sid, float *action) {
	ai_t *ai = ZRC_GET(zrc, ai, id);
	if (!ai) return;

	sense_t *sense = ZRC_GET(zrc, sense, id);
	if (!sid) return;

	physics_t *physics = ZRC_GET(zrc, physics, id);
	if (!physics) return;

	physics_t *sphysics = ZRC_GET(zrc, physics, sid);
	if (!sphysics) return;

	cpVect front = cpvforangle(physics->angle);
	cpVect sfront = cpvforangle(sphysics->angle);

	int op = 0;
	float distances[AI_LIDAR];
	for (int i = 0; i < AI_LIDAR; ++i) {
		float a = ((float)i / AI_LIDAR) * (CP_PI * 2);
		cpVect v = cpvforangle(a);
		v = cpvrotate(v, front);
		float oa = cpvdot(v, sfront);
		ai->locomotion_obs[op++] = oa * action[0];

		cpVect p = cpvadd(physics->position, v);
		float d = cpvdistsq(p, sphysics->position);
		distances[i] = d;
	}

	float min_distance = FLT_MAX;
	float max_distance = -FLT_MAX;
	for (int i = 0; i < AI_LIDAR; ++i) {
		min_distance = min(min_distance, distances[i]);
		max_distance = max(max_distance, distances[i]);
	}
	for (int i = 0; i < AI_LIDAR; ++i) {
		double d = (distances[i] - min_distance) / (max_distance - min_distance);
		ai->locomotion_obs[op++] = (float)(action[1] / (1+d));
	}

	assert(op == AI_LOCOMOTION_OBS_LENGTH);
}

void ai_act_sense(zrc_t *zrc, id_t id, id_t sid, float *action) {
	ai_t *ai = ZRC_GET(zrc, ai, id);
	if (!ai) return;

	sense_t *sense = ZRC_GET(zrc, sense, id);
	if (!sid) return;

	for (int i = 0; i < AI_SENSE_ACT_LENGTH; ++i) {
		// never cleared, would have to store per-sensed
		//ai->locomotion_obs[i] += action[i];

		//ai->locomotion_obs[i] = cpflerp(ai->locomotion_obs[i], action[i], 1.0f / max(1, sense->num_entities));
		ai->locomotion_obs[i] = action[i];
	}
}

void ai_act_locomotion(zrc_t *zrc, id_t id, float *action) {
	for (int i = 0; i < AI_LOCOMOTION_ACT_LENGTH; ++i) {
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
	zrc_assert(ap <= AI_LOCOMOTION_ACT_LENGTH);
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
