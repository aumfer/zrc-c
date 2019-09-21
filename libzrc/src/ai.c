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
static void ai_reward_inbounds(zrc_t *zrc, id_t id, ai_t *ai) {
	physics_t *physics = ZRC_GET(zrc, physics, id);
	if (physics) {
		if (physics->position.x < 0 || physics->position.x > WORLD_SIZE || physics->position.y < 0 || physics->position.y > WORLD_SIZE) {
			ai->reward -= TICK_RATE;
		}
	}
}
static void ai_reward_fight(zrc_t *zrc, id_t id, ai_t *ai) {
	float next_reward = 0;

	physics_t *physics = ZRC_GET(zrc, physics, id);
	sense_t *sense = ZRC_GET(zrc, sense, id);
	life_t *life = ZRC_GET(zrc, life, id);
	team_t *team = ZRC_GET(zrc, team, id);

	// reward for damage dealt
	damage_dealt_t *damage_dealt;
	ZRC_RECEIVE(zrc, damage_dealt, id, &ai->damage_dealt_index, damage_dealt, {
		team_t *tteam = ZRC_GET(zrc, team, damage_dealt->to);
		life_t *tlife = ZRC_GET(zrc, life, damage_dealt->to);
		if (team && tteam && *team == *tteam) {
			next_reward -= damage_dealt->health;
		} else {
			next_reward += damage_dealt->health * (1 / max(0.1f, life->health / life->max_health));
		}
	});

	id_t *killed;
	ZRC_RECEIVE(zrc, got_kill, id, &ai->got_kill_index, killed, {
		team_t *tteam = ZRC_GET(zrc, team, *killed);
		if (team && tteam && *team == *tteam) {
			next_reward -= 1000;
		} else {
			next_reward += 1000;
		}
	});

	if (life) {
		damage_t *damage;
		ZRC_RECEIVE(zrc, damage, id, &ai->damage_taken_index, damage, {
			next_reward -= damage->health * (1 / max(0.1f, life->health / life->max_health));
		});
	}

	//next_reward /= 1000;
	ai->reward += next_reward;

	ai->total_reward += ai->reward;
}

static void ai_reward_seekalign(zrc_t *zrc, id_t id, ai_t *ai) {
	if (!ZRC_HAS(zrc, physics, id) || !ZRC_HAD(zrc, physics, id)) return;

	float reward = 0;

	physics_t *physics = ZRC_GET_READ(zrc, physics, id);
	physics_t *pphysics = ZRC_GET_PREV(zrc, physics, id);

	cpVect goalp = ai->train_seekalign.goalp;
	float goala = ai->train_seekalign.goala;
	
	cpVect front = cpvforangle(physics->angle);
	cpVect pfront = cpvforangle(pphysics->angle);
	cpVect goal_front = cpvforangle(goala);

	float ad = cpvdot(front, goal_front);
	float pad = cpvdot(pfront, goal_front);
	float ar = (ad - pad) / CP_PI / max(1, physics->max_spin);
	reward += ar;
	
	cpVect move = cpvsub(physics->position, pphysics->position);
	if (move.x && move.y) {
		float dmove = cpvlength(move);
		cpVect dir = cpvmult(move, 1 / dmove);
		cpVect gmove = cpvsub(goalp, pphysics->position);
		cpVect gdir = gmove.x && gmove.y ? cpvnormalize(gmove) : cpvzero;
		float pr = cpvdot(dir, gdir) * dmove / max(1, physics->max_speed);
		reward += pr;
	}
	
	float pd = cpvdist(goalp, physics->position);
	if (pd < physics->radius && ad > 0.8f) {
		float speed = cpvlength(physics->velocity);
		reward += 10 * (1 - (speed / max(1, physics->max_speed)));
	}

	ai->reward = reward;
	ai->total_reward += reward;
}

static void ai_reward_followalign(zrc_t *zrc, id_t id, ai_t *ai) {
	if (!ZRC_HAS(zrc, physics, id) || !ZRC_HAD(zrc, physics, id)) return;

	float reward = 0;

	physics_t *physics = ZRC_GET_READ(zrc, physics, id);
	physics_t *pphysics = ZRC_GET_PREV(zrc, physics, id);

	sense_t *sense = ZRC_GET_READ(zrc, sense, id);
	if (sense->num_entities) {

		float mind = FLT_MAX;
		cpVect goalp;
		float goala;
		for (int i = 0; i < sense->num_entities; ++i) {
			physics_t *sphysics = ZRC_GET_READ(zrc, physics, sense->entities[i]);
			float sd = cpvdistsq(physics->position, sphysics->position);
			if (!i || sd < mind) {
				goalp = sphysics->position;
				goala = sphysics->angle;
			}
		}

		cpVect front = cpvforangle(physics->angle);
		cpVect pfront = cpvforangle(pphysics->angle);
		cpVect goal_front = cpvforangle(goala);

		float ad = cpvdot(front, goal_front);
		float pad = cpvdot(pfront, goal_front);
		float ar = (ad - pad) / CP_PI / max(1, physics->max_spin);
		// todo align
		//reward += ar;

		cpVect move = cpvsub(physics->position, pphysics->position);
		if (move.x && move.y) {
			float dmove = cpvlength(move);
			cpVect dir = cpvmult(move, 1 / dmove);
			cpVect gmove = cpvsub(goalp, pphysics->position);
			cpVect gdir = gmove.x && gmove.y ? cpvnormalize(gmove) : cpvzero;
			float pr = cpvdot(dir, gdir) * dmove / max(1, physics->max_speed);
			reward += pr;
		}

		float pd = cpvdist(goalp, physics->position);
		if (pd < physics->radius && ad > 0.8f) {
			float speed = cpvlength(physics->velocity);
			reward += 10 * (1 - (speed / max(1, physics->max_speed)));
		}
	}

	ai->reward = reward;
	ai->total_reward += reward;
}

void ai_update(zrc_t *zrc, id_t id, ai_t *ai) {
	ai->reward = 0;
	ai_reward_inbounds(zrc, id, ai);
	if ((ai->reward_flags & AI_REWARD_SEEKALIGN) == AI_REWARD_SEEKALIGN) {
		ai_reward_seekalign(zrc, id, ai);
	}
	if ((ai->reward_flags & AI_REWARD_FOLLOWALIGN) == AI_REWARD_FOLLOWALIGN) {
		ai_reward_seekalign(zrc, id, ai);
	}
	if ((ai->reward_flags & AI_REWARD_FIGHT) == AI_REWARD_FIGHT) {
		ai_reward_fight(zrc, id, ai);
	}
}

void ai_observe_locomotion(zrc_t *zrc, id_t id, float *observation) {
	const ai_t *ai = ZRC_GET(zrc, ai, id);
	if (!ai) return;

	memcpy(observation, ai->sense_act, sizeof(ai->sense_act));

	ai_t *write = ZRC_GET_WRITE(zrc, ai, id);
	memcpy(write->locomotion_obs, ai->sense_act, sizeof(write->locomotion_obs));
}

void ai_observe_locomotion_seekalign(zrc_t *zrc, id_t id, float *observation) {
	const ai_t *ai = ZRC_GET(zrc, ai, id);
	if (!ai) {
		return;
	}
	const physics_t *physics = ZRC_GET_READ(zrc, physics, id);

	cpVect goalp = ai->train_seekalign.goalp;
	float goala = ai->train_seekalign.goala;

	cpVect front = cpvforangle(physics->angle);
	cpVect goal_front = cpvforangle(goala);
	float goald = cpvdistsq(physics->position, goalp);

	int op = 0;
	float distances[AI_LIDAR];
	for (int i = 0; i < AI_LIDAR; ++i) {
		float a = ((float)i / AI_LIDAR) * (CP_PI * 2);
		a -= CP_PI; // put front in middle of lidar
		cpVect v = cpvforangle(a);
		v = cpvrotate(v, front);
		float oa = cpvdot(v, goal_front);
		observation[op++] = oa;

		cpVect dp = cpvmult(v, 0.1f); // too small = floating point issues? too big = overshoot
		cpVect p = cpvadd(physics->position, dp);
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
		double d = max_distance != min_distance ? (distances[i] - min_distance) / (max_distance - min_distance) : 0;
		observation[op++] = (float)snorm(d);
	}

	memcpy(&observation[op], ai->locomotion_act, sizeof(ai->locomotion_act));
	op += AI_LOCOMOTION_ACT_LENGTH;

	assert(op == AI_LOCOMOTION_OBS_LENGTH);

	ai_t *write = ZRC_GET_WRITE(zrc, ai, id);
	memcpy(write->locomotion_obs, observation, sizeof(write->locomotion_obs));
}

void ai_observe_sense(zrc_t *zrc, id_t id, float *observation) {
	const ai_t *ai = ZRC_GET_READ(zrc, ai, id);
	const physics_t *physics = ZRC_GET_READ(zrc, physics, id);
	const sense_t *sense = ZRC_GET_READ(zrc, sense, id);
	const team_t *team = ZRC_GET(zrc, team, id);
	int op = 0;

	cpVect front = cpvforangle(physics->angle);
	float range = max(1, sense->range);

	float lidar[AI_SENSE_ENTITY_LENGTH][AI_LIDAR] = { 0 };
	for (int i = 0; i < sense->num_entities; ++i) {
			id_t sid = sense->entities[i];
			if (sid == id) continue;

			const physics_t *sphysics = ZRC_GET_READ(zrc, physics, sid);
			const caster_t *scaster = ZRC_GET_READ(zrc, caster, sid);
			const life_t *slife = ZRC_GET_READ(zrc, life, sid);
			const team_t *steam = ZRC_GET(zrc, team, sid);

			const ttl_t *ttl = ZRC_GET(zrc, ttl, sid);
			if (ttl) continue; // debug temp ignore projectiles

			cpVect goal_front = cpvforangle(sphysics->angle);
			cpVect goalp = sphysics->position;
			float goald = cpvdistsq(physics->position, goalp);

			for (int i = 0; i < AI_LIDAR; ++i) {
				float a = ((float)i / AI_LIDAR) * (CP_PI * 2);
				a -= CP_PI; // put front in middle of lidar
				cpVect v = cpvforangle(a);
				v = cpvrotate(v, front);
				float oa = cpvdot(v, goal_front);
				lidar[0][i] = max(lidar[0][i], oa);

				cpVect dp = cpvmult(v, 0.1f); // too small = floating point issues? too big = overshoot
				cpVect p = cpvadd(physics->position, dp);
				float d = cpvdistsq(p, goalp);
				lidar[1][i] = max(lidar[1][i], (goald - d));
			}

			//{
			//	cpVect soffset = cpvsub(sphysics->position, physics->position);
			//	cpVect srel_offset = cpvrotate(soffset, front);
			//	cpVect sdir = cpvnormalize(srel_offset);
			//	float a = cpvtoangle(sdir);
			//	int li = (int)(((a + CP_PI) / (2 * CP_PI)) * AI_LIDAR);
			//
			//	float myteam = team && steam ? (*team == *steam ? +1.0f : -1.0f) : 0.0f;
			//	lidar[2][li] = max(lidar[2][li], myteam);
			//}
	}

	float mins[AI_SENSE_ENTITY_LENGTH];
	float maxs[AI_SENSE_ENTITY_LENGTH];
	for (int i = 0; i < AI_SENSE_ENTITY_LENGTH; ++i) {
		mins[i] = FLT_MAX;
		maxs[i] = -FLT_MAX;
	}
	for (int i = 0; i < AI_LIDAR; ++i) {
		for (int j = 0; j < AI_SENSE_ENTITY_LENGTH; ++j) {
			mins[j] = min(mins[j], lidar[j][i]);
			maxs[j] = max(maxs[j], lidar[j][i]);
		}
	}

	for (int i = 0; i < AI_LIDAR; ++i) {
		// skip known normalized
		for (int j = 1; j < AI_SENSE_ENTITY_LENGTH; ++j) {
			double d = mins[j] != maxs[j] ? (lidar[j][i] - mins[j]) / (maxs[j] - mins[j]) : cpfclamp(lidar[j][i], -1, +1);
			lidar[1][i] = (float)snorm(d);
		}
	}
	memcpy(&observation[op], lidar, sizeof(lidar));
	op += AI_LIDAR * AI_SENSE_ENTITY_LENGTH;

	//memcpy(&observation[op], ai->sense_act, sizeof(ai->sense_act));
	//op += AI_SENSE_ACT_LENGTH;

	zrc_assert(op == AI_SENSE_OBS_LENGTH);

	ai_t *write = ZRC_GET_WRITE(zrc, ai, id);
	memcpy(write->sense_obs, observation, sizeof(write->sense_obs));
}

static unsigned has_cast[ABILITY_COUNT];

static float step(float v) {
	// analog (gamepad)
	//return cpfclamp(v * 2, -1, +1);
	// digital (keyboard)
	return v < -0.2f ? -1.0f : (v > 0.2f ? +1.0f : 0.0f);
	//return v < 0 ? -1.0f : (v > 0 ? +1.0f : 0.0f);
}

void ai_act_sense(zrc_t *zrc, id_t id, float *action) {
	physics_t *physics = ZRC_GET_READ(zrc, physics, id);

	//if (zrc->frame > 3/TICK_RATE) {
	//	cast_t cast = {
	//		.caster_ability = 1,
	//		.cast_flags = CAST_WANTCAST,
	//	};
	//	ZRC_SEND(zrc, cast, id, &cast);
	//}

	ai_t *ai = ZRC_GET_WRITE(zrc, ai, id);
	memcpy(ai->sense_act, action, sizeof(ai->sense_act));
}

void ai_act_locomotion(zrc_t *zrc, id_t id, float *action) {
	for (int i = 0; i < AI_LOCOMOTION_ACT_LENGTH; ++i) {
		if (!isvalid(action[i])) {
			action[i] = 0;
			//zrc_assert(0);
		}
		zrc_assert(isvalid(action[i]));
	}
	ai_t *ai = ZRC_GET_WRITE(zrc, ai, id);
	physics_t *physics = ZRC_GET_WRITE(zrc, physics, id);
	if (!physics || !ai) return;

	int ap = 0;
	flight_thrust_t flight_thrust = {
		.thrust = { step(action[ap++]), step(action[ap++]) },
		.turn = step(action[ap++])
	};
	//flight_thrust.thrust[0] = step(flight_thrust.thrust[0]);
	//flight_thrust.thrust[1] = step(flight_thrust.thrust[1]);
	//flight_thrust.turn = step(flight_thrust.turn);
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
	memcpy(ai->locomotion_act, action, sizeof(ai->locomotion_act));
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
