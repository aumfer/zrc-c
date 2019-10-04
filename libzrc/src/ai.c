#include <zrc.h>
#include <zmath.h>
#include <string.h>
#include <stdio.h>

static float ability_vector[ABILITY_COUNT][ABILITY_COUNT];

static ability_id_t ability_match(const rl_t *ai, float *v);

#define isvalid(v) (!v || isnormal(v))

void rl_startup(zrc_t *zrc) {
	//printf("ai %zu\n", sizeof(zrc->ai));
	for (int i = 0; i < ABILITY_COUNT; ++i) {
		ability_vector[i][i] = 1;
	}
	//printf("AI_ACT_LENGTH %u\n", AI_ACT_LENGTH);
	//printf("AI_OBS_LENGTH %u\n", AI_OBS_LENGTH);
}
void rl_shutdown(zrc_t *zrc) {

}
void rl_create(zrc_t *zrc, id_t id, rl_t *ai) {
}
void rl_delete(zrc_t *zrc, id_t id, rl_t *ai) {
}
static float rl_reward_inbounds(zrc_t *zrc, id_t id, rl_t *ai) {
	physics_t *physics = ZRC_GET(zrc, physics, id);
	float reward = 0;
	if (physics) {
		if (physics->position.x < -WORLD_HALF || physics->position.x > WORLD_HALF || physics->position.y < -WORLD_HALF || physics->position.y > WORLD_HALF) {
			reward -= TICK_RATE;
		}
	}
	return reward;
}
static float rl_reward_fight(const zrc_t *zrc, id_t id, rl_t *ai) {
	const physics_t *physics = ZRC_GET(zrc, physics, id);
	const sense_t *sense = ZRC_GET(zrc, sense, id);
	const life_t *life = ZRC_GET(zrc, life, id);
	const team_t *team = ZRC_GET(zrc, team, id);

	float reward = 0;

	// reward for damage dealt
	const damage_dealt_t *damage_dealt;
	ZRC_RECEIVE(zrc, damage_dealt, id, &ai->recv_damage_dealt, damage_dealt, {
		const team_t *tteam = ZRC_GET(zrc, team, damage_dealt->to);
		const life_t *tlife = ZRC_GET(zrc, life, damage_dealt->to);
		if (team && tteam && *team == *tteam) {
			reward -= damage_dealt->health;
		} else {
			reward += damage_dealt->health * (1 / max(0.1f, life->health / life->max_health));
		}
	});

	const id_t *killed;
	ZRC_RECEIVE(zrc, got_kill, id, &ai->recv_got_kill, killed, {
		const team_t *tteam = ZRC_GET(zrc, team, *killed);
		if (team && tteam && *team == *tteam) {
			reward -= 1000;
		} else {
			reward += 1000;
		}
	});

	if (life) {
		const damage_t *damage;
		ZRC_RECEIVE(zrc, damage, id, &ai->recv_damage_taken, damage, {
			reward -= damage->health * (1 / max(0.1f, life->health / life->max_health));
		});
	}

	return reward;
}

static float rl_reward_seekalign(zrc_t *zrc, id_t id, rl_t *ai) {
	float reward = 0;

	if (!ZRC_HAS(zrc, physics, id) || !ZRC_HAD(zrc, physics, id)) return reward;

	const physics_t *physics = ZRC_GET_READ(zrc, physics, id);
	const physics_t *pphysics = ZRC_GET_PREV(zrc, physics, id);

	cpVect goalp = ai->train.goalp;
	float goala = ai->train.goala;
	
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
		float spin = fabsf(physics->angular_velocity);
		reward += 1 * (1 - (speed / max(1, physics->max_speed))) * (1 - (spin / max(1, physics->max_spin)));

		ai->train.goalp.x = randfs() * WORLD_HALF / 2;
		ai->train.goalp.y = randfs() * WORLD_HALF / 2;
		ai->train.goala = randf() * 2 * CP_PI;

		visual_t *v = ZRC_GET_WRITE(zrc, visual, ai->train.goalid);
		if (v) {
			v->position[0] = ai->train.goalp.x;
			v->position[1] = ai->train.goalp.y;
			v->angle = ai->train.goala;
		}
	} else {
		//reward -= TICK_RATE;
	}

	return reward;
}

static float rl_reward_followalign(const zrc_t *zrc, id_t id, rl_t *ai) {
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
		reward += ar;

		cpVect move = cpvsub(physics->position, pphysics->position);
		if (move.x && move.y) {
			float dmove = cpvlength(move);
			cpVect dir = cpvmult(move, 1 / dmove);
			cpVect gmove = cpvsub(goalp, pphysics->position);
			cpVect gdir = gmove.x && gmove.y ? cpvnormalize(gmove) : cpvzero;
			float pr = cpvdot(dir, gdir) * (dmove / max(1, physics->max_speed));
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

void rl_update(zrc_t *zrc, id_t id, rl_t *ai) {
	ai->reward = 0;
	ai->reward += rl_reward_inbounds(zrc, id, ai);
	if ((ai->reward_flags & RL_REWARD_SEEKALIGN) == RL_REWARD_SEEKALIGN) {
		ai->reward += rl_reward_seekalign(zrc, id, ai);
	}
	if ((ai->reward_flags & RL_REWARD_FOLLOWALIGN) == RL_REWARD_FOLLOWALIGN) {
		ai->reward += rl_reward_seekalign(zrc, id, ai);
	}
	if ((ai->reward_flags & RL_REWARD_FIGHT) == RL_REWARD_FIGHT) {
		ai->reward += rl_reward_fight(zrc, id, ai);
	}
	ai->total_reward += ai->reward;
}
static void rl_observe_command(const zrc_t *zrc, id_t id, struct rl_obs_command *observation) {
	const physics_t *physics = ZRC_GET(zrc, physics, id);
	const rl_t *ai = ZRC_GET(zrc, rl, id);
	if (!physics || !ai) return;

	cpVect goalp = ai->train.goalp;
	float goala = ai->train.goala;

	cpVect front = physics->front;
	cpVect goal_front = cpvforangle(goala);
	cpVect goalo = cpvsub(goalp, physics->position);
	float goald = cpvlength(goalo);
	cpVect goalon = cpvmult(goalo, 1 / max(1, goald));
	goalon = cpvnormalize(goalo);
	//goalon = cpvrotate(goalon, front);
	float goaloa = cpvtoangle(goalon);
	goaloa = physics->angle - goaloa;
	goaloa = wrapMinMax(goaloa, -CP_PI, CP_PI);

	//goala = physics->angle - goala;
	//goala = wrapMinMax(goala, -CP_PI, CP_PI);

	float distances[RL_LIDAR] = { 0 };
	float alignments[RL_LIDAR] = { 0 };
	for (int i = 0; i < RL_LIDAR; ++i) {
		float a = ((float)i / RL_LIDAR) * (CP_PI * 2);
		a -= CP_PI; // put front in middle of lidar
		a += physics->angle;
		cpVect v = cpvforangle(a);
		//v = cpvrotate(v, front);

		//float lookahead = 0.1f; // too small = floating point issues? too big = overshoot
		float lookahead = physics->radius;
		cpVect dp = cpvmult(v, lookahead);
		cpVect p = cpvadd(physics->position, dp);
		float d = cpvdist(p, goalp);
		distances[i] = goald - d;

		float oa = cpvdot(v, goal_front);
		alignments[i] = oa;
	}

	float min_distance = FLT_MAX;
	float max_distance = -FLT_MAX;
	for (int i = 0; i < RL_LIDAR; ++i) {
		min_distance = min(min_distance, distances[i]);
		max_distance = max(max_distance, distances[i]);
	}
	for (int i = 0; i < RL_LIDAR; ++i) {
		double d = max_distance != min_distance ? (distances[i] - min_distance) / (max_distance - min_distance) : 0;
		assert(isvalid(d));
		observation->dist[i] = (float)snorm(d);
	}
	for (int i = 0; i < RL_LIDAR; ++i) {
		assert(isvalid(alignments[i]));
		observation->align[i] = alignments[i];
	}
}
#if 0
static void rl_observe_sense(const zrc_t *zrc, id_t id, id_t sid, struct rl_obs_entity *observation) {
	const physics_t *physics = ZRC_GET(zrc, physics, id);
	const physics_t *sphysics = ZRC_GET(zrc, physics, sid);
	if (!physics || !sphysics) return;

	cpVect goalp = sphysics->position;
	float goala = sphysics->angle;

	cpVect front = physics->front;
	cpVect goal_front = cpvforangle(goala);
	cpVect goalo = cpvsub(goalp, physics->position);
	float goald = cpvlength(goalo);
	cpVect goalon = cpvmult(goalo, 1 / max(1, goald));
	goalon = cpvnormalize(goalo);
	//goalon = cpvrotate(goalon, front);
	float goaloa = cpvtoangle(goalon);
	goaloa = physics->angle - goaloa;
	goaloa = wrapMinMax(goaloa, -CP_PI, CP_PI);

	//goala = physics->angle - goala;
	//goala = wrapMinMax(goala, -CP_PI, CP_PI);

	float distances[AI_LIDAR] = { 0 };
	float alignments[AI_LIDAR] = { 0 };
	for (int i = 0; i < AI_LIDAR; ++i) {
		float a = ((float)i / AI_LIDAR) * (CP_PI * 2);
		a -= CP_PI; // put front in middle of lidar
		a += physics->angle;
		cpVect v = cpvforangle(a);
		//v = cpvrotate(v, front);

		//float lookahead = 0.1f; // too small = floating point issues? too big = overshoot
		float lookahead = physics->radius;
		cpVect dp = cpvmult(v, lookahead);
		cpVect p = cpvadd(physics->position, dp);
		float d = cpvdist(p, goalp);
		distances[i] = goald - d;

		float oa = cpvdot(v, goal_front);
		alignments[i] = oa;
	}

	float min_distance = FLT_MAX;
	float max_distance = -FLT_MAX;
	for (int i = 0; i < AI_LIDAR; ++i) {
		min_distance = min(min_distance, distances[i]);
		max_distance = max(max_distance, distances[i]);
	}
	for (int i = 0; i < AI_LIDAR; ++i) {
		double d = max_distance != min_distance ? (distances[i] - min_distance) / (max_distance - min_distance) : 0;
		assert(isvalid(d));
		observation->dist[i] = (float)snorm(d);
	}
	for (int i = 0; i < AI_LIDAR; ++i) {
		assert(isvalid(alignments[i]));
		observation->align[i] = alignments[i];
	}

	if (sid != ID_INVALID) {
		const team_t *team = ZRC_GET(zrc, team, id);
		const team_t *steam = ZRC_GET(zrc, team, sid);
		observation->team = team && steam ? (*team == *steam ? +1.0f : -1.0f) : 0.0f;
	}
}
#endif
void rl_observe(const zrc_t *zrc, id_t id, rl_obs_t *observation) {
	const rl_t *ai = ZRC_GET(zrc, rl, id);
	const sense_t *sense = ZRC_GET(zrc, sense, id);
	if (!sense || !ai) return;

	rl_observe_command(zrc, id, &observation->command);
	//for (int i = 0; i < SENSE_MAX_ENTITIES; ++i) {
	//	if (i < sense->num_entities) {
	//		id_t sid = sense->entities[i];
	//		if (sid == id) continue;
	//		ai_observe_sense(zrc, id, sid, &observation->sense[i]);
	//	} else {
	//		memset(&observation->sense[i], 0, sizeof(observation->sense[i]));
	//	}
	//}

	observation->act = ai->act;
}

static unsigned has_cast[ABILITY_COUNT];

static float step(float v) {
	assert(isvalid(v));
	// analog (gamepad)
	//return cpfclamp(v * 2, -1, +1);
	// digital (keyboard)
	//return v < -0.2f ? -1.0f : (v > 0.2f ? +1.0f : 0.0f);
	//return v < 0 ? -1.0f : (v > 0 ? +1.0f : 0.0f);
	return v;
}

void rl_act(zrc_t *zrc, id_t id, rl_act_t *action) {
	rl_t *ai = ZRC_GET_WRITE(zrc, rl, id);
	physics_t *physics = ZRC_GET_WRITE(zrc, physics, id);
	if (!physics || !ai) return;

	flight_thrust_t flight_thrust = {
		.thrust = { step(action->thrust[0]), step(action->thrust[1]) },
		.turn = step(action->turn)
	};
	//flight_thrust.thrust[0] = step(flight_thrust.thrust[0]);
	//flight_thrust.thrust[1] = step(flight_thrust.thrust[1]);
	//flight_thrust.turn = step(flight_thrust.turn);

	int dry = action->damp > 0;
	if (dry) {
		physics->damping = 0;
	} else {
		physics->damping = SHIP_DAMPING;
	}
	ZRC_SEND(zrc, flight_thrust, id, &flight_thrust);

	//if (action[ap++] > 0.2f) {
	//	cast_t cast = {
	//		.caster_ability = 1,
	//		.cast_flags = CAST_WANTCAST,
	//	};
	//	ZRC_SEND(zrc, cast, id, &cast);
	//}
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

	ai->act = *action;
}


static ability_id_t ability_match(const rl_t *ai, float *v) {
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
