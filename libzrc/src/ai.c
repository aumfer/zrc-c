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
static float ai_reward_inbounds(zrc_t *zrc, id_t id, ai_t *ai) {
	physics_t *physics = ZRC_GET(zrc, physics, id);
	float reward = 0;
	if (physics) {
		if (physics->position.x < -WORLD_HALF || physics->position.x > WORLD_HALF || physics->position.y < -WORLD_HALF || physics->position.y > WORLD_HALF) {
			reward -= TICK_RATE;
		}
	}
	return reward;
}
static float ai_reward_fight(const zrc_t *zrc, id_t id, ai_t *ai) {
	const physics_t *physics = ZRC_GET(zrc, physics, id);
	const sense_t *sense = ZRC_GET(zrc, sense, id);
	const life_t *life = ZRC_GET(zrc, life, id);
	const team_t *team = ZRC_GET(zrc, team, id);

	float reward = 0;

	// reward for damage dealt
	const damage_dealt_t *damage_dealt;
	ZRC_RECEIVE(zrc, damage_dealt, id, &ai->damage_dealt_index, damage_dealt, {
		const team_t *tteam = ZRC_GET(zrc, team, damage_dealt->to);
		const life_t *tlife = ZRC_GET(zrc, life, damage_dealt->to);
		if (team && tteam && *team == *tteam) {
			reward -= damage_dealt->health;
		} else {
			reward += damage_dealt->health * (1 / max(0.1f, life->health / life->max_health));
		}
	});

	const id_t *killed;
	ZRC_RECEIVE(zrc, got_kill, id, &ai->got_kill_index, killed, {
		const team_t *tteam = ZRC_GET(zrc, team, *killed);
		if (team && tteam && *team == *tteam) {
			reward -= 1000;
		} else {
			reward += 1000;
		}
	});

	if (life) {
		const damage_t *damage;
		ZRC_RECEIVE(zrc, damage, id, &ai->damage_taken_index, damage, {
			reward -= damage->health * (1 / max(0.1f, life->health / life->max_health));
		});
	}

	return reward;
}

static float ai_reward_seekalign(const zrc_t *zrc, id_t id, ai_t *ai) {
	float reward = 0;

	if (!ZRC_HAS(zrc, physics, id) || !ZRC_HAD(zrc, physics, id)) return reward;

	const physics_t *physics = ZRC_GET_READ(zrc, physics, id);
	const physics_t *pphysics = ZRC_GET_PREV(zrc, physics, id);

	cpVect goalp = ai->train_seekalign.goalp;
	float goala = ai->train_seekalign.goala;
	
	cpVect front = physics->front;
	cpVect pfront = pphysics->front;
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

	return reward;
}

static float ai_reward_followalign(const zrc_t *zrc, id_t id, ai_t *ai) {
	float reward = 0;

	if (!ZRC_HAS(zrc, physics, id) || !ZRC_HAD(zrc, physics, id)) return reward;

	const physics_t *physics = ZRC_GET_READ(zrc, physics, id);
	const physics_t *pphysics = ZRC_GET_PREV(zrc, physics, id);

	const sense_t *sense = ZRC_GET_READ(zrc, sense, id);
	if (sense->num_entities) {

		float mind = FLT_MAX;
		cpVect goalp;
		float goala;
		for (int i = 0; i < sense->num_entities; ++i) {
			const physics_t *sphysics = ZRC_GET_READ(zrc, physics, sense->entities[i]);
			float sd = cpvdistsq(physics->position, sphysics->position);
			if (!i || sd < mind) {
				goalp = sphysics->position;
				goala = sphysics->angle;
			}
		}

		cpVect front = physics->front;
		cpVect pfront = pphysics->front;
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

	return reward;
}

void ai_update(zrc_t *zrc, id_t id, ai_t *ai) {
	ai->reward = 0;
	ai->reward += ai_reward_inbounds(zrc, id, ai);
	if ((ai->reward_flags & AI_REWARD_SEEKALIGN) == AI_REWARD_SEEKALIGN) {
		ai->reward += ai_reward_seekalign(zrc, id, ai);
	}
	if ((ai->reward_flags & AI_REWARD_FOLLOWALIGN) == AI_REWARD_FOLLOWALIGN) {
		ai->reward += ai_reward_seekalign(zrc, id, ai);
	}
	if ((ai->reward_flags & AI_REWARD_FIGHT) == AI_REWARD_FIGHT) {
		ai->reward += ai_reward_fight(zrc, id, ai);
	}
	ai->total_reward += ai->reward;
}

void ai_observe(zrc_t *zrc, id_t id, float *observation) {
	const ai_t *ai = ZRC_GET_READ(zrc, ai, id);
	const physics_t *physics = ZRC_GET_READ(zrc, physics, id);
	const sense_t *sense = ZRC_GET_READ(zrc, sense, id);
	const team_t *team = ZRC_GET(zrc, team, id);
	int op = 0;

	cpVect front = physics->front;
	float range = max(1, sense->range);

	float distances[AI_LIDAR] = { 0 };

	float lidar[AI_OBS_ENTITY_LENGTH][AI_LIDAR] = { 0 };
	for (int i = 0; i < sense->num_entities; ++i) {
		id_t sid = sense->entities[i];
		if (sid == id) continue;

		const physics_t *sphysics = ZRC_GET_READ(zrc, physics, sid);
		const caster_t *scaster = ZRC_GET_READ(zrc, caster, sid);
		const life_t *slife = ZRC_GET_READ(zrc, life, sid);
		const team_t *steam = ZRC_GET(zrc, team, sid);

		const ttl_t *ttl = ZRC_GET(zrc, ttl, sid);
		if (ttl) continue; // debug temp ignore projectiles

		cpVect sfront = sphysics->front;
		float sd = cpvdist(physics->position, sphysics->position);
		sd /= range;
		sd = snorm(sd);

		cpVect soffset = cpvsub(sphysics->position, physics->position);
		cpVect sdir = cpvnormalize(soffset);
		cpVect srel_dir = cpvrotate(sdir, front);
		float a = cpvtoangle(srel_dir);
		int li = (int)(unorm(a / CP_PI) * AI_LIDAR);
		li = clamp(li, 0, AI_LIDAR-1);
		assert(li >= 0);
		assert(li < AI_LIDAR);

		cpVect v = cpvforangle(a);
		v = cpvrotate(v, front);

		//float oa = cpvdot(v, goal_front);
		//float oa = cpvdot(front, sdir);
		float oa = (sphysics->angle - physics->angle) / CP_PI;

		float ot = team && steam ? (*team == *steam ? +1.0f : -1.0f) : 0.0f;

		if (!distances[li] || sd < distances[li]) {
			distances[li] = sd;
			lidar[0][li] = sd;
			lidar[1][li] = ot;
			lidar[2][li] = oa;
		}
	}

	memcpy(&observation[op], lidar, sizeof(lidar));
	op += AI_LIDAR * AI_OBS_ENTITY_LENGTH;

	//memcpy(&observation[op], ai->sense_act, sizeof(ai->sense_act));
	//op += AI_SENSE_ACT_LENGTH;

	memcpy(&observation[op], ai->locomotion_act, sizeof(ai->locomotion_act));
	op += AI_ACT_LENGTH;

	zrc_assert(op == AI_OBS_LENGTH);

	ai_t *write = ZRC_GET_WRITE(zrc, ai, id);
	memcpy(write->locomotion_obs, observation, sizeof(write->locomotion_obs));
}

static unsigned has_cast[ABILITY_COUNT];

static float step(float v) {
	// analog (gamepad)
	//return cpfclamp(v * 2, -1, +1);
	// digital (keyboard)
	return v < -0.2f ? -1.0f : (v > 0.2f ? +1.0f : 0.0f);
	//return v < 0 ? -1.0f : (v > 0 ? +1.0f : 0.0f);
}

void ai_act(zrc_t *zrc, id_t id, float *action) {
	for (int i = 0; i < AI_ACT_LENGTH; ++i) {
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

	if (action[ap++] > 0.2f) {
		cast_t cast = {
			.caster_ability = 1,
			.cast_flags = CAST_WANTCAST,
		};
		ZRC_SEND(zrc, cast, id, &cast);
	}
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

	zrc_assert(ap <= AI_ACT_LENGTH);
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
