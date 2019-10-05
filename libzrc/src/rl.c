#include <zrc.h>
#include <zmath.h>
#include <string.h>
#include <stdio.h>

static float ability_vector[ABILITY_COUNT][ABILITY_COUNT];

static ability_id_t ability_match(const rl_t *rl, float *v);

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
void rl_create(zrc_t *zrc, id_t id, rl_t *rl) {
}
void rl_delete(zrc_t *zrc, id_t id, rl_t *rl) {
}
static float step(float v) {
	//if (!isvalid(v)) return 0;
	assert(isvalid(v));
	// analog (gamepad)
	//return cpfclamp(v * 2, -1, +1);
	// digital (keyboard)
	//return v < -0.2f ? -1.0f : (v > 0.2f ? +1.0f : 0.0f);
	//return v < 0 ? -1.0f : (v > 0 ? +1.0f : 0.0f);
	return v;
}
void rl_update(zrc_t *zrc, id_t id, rl_t *rl) {
	const physics_t *physics = ZRC_GET(zrc, physics, id);
	assert(physics);
	if (!physics) return;

	locomotion_behavior_t behaviors[max_locomotion_behavior_messages];
	int num_behaviors = 0;
	locomotion_behavior_t *locomotion_behavior;
	ZRC_RECEIVE(zrc, locomotion_behavior, id, &rl->recv_locomotion_behavior, locomotion_behavior, {
		behaviors[num_behaviors++] = *locomotion_behavior;
	});

	double potentials[8][8];
	double current;
	double minp = FLT_MAX;
	double maxp = -FLT_MAX;
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			float move_angle = snorm(i / 8.0f) * CP_PI;
			move_angle += physics->angle;
			float turn_angle = snorm(j / 8.0f) * CP_PI;
			turn_angle += physics->angle;
			cpVect move_dir = cpvforangle(move_angle);
			cpVect move = cpvmult(move_dir, physics->radius);
			cpVect point = cpvadd(physics->position, move);
			cpVect front = cpvforangle(turn_angle);

			double potential = 0;
			for (int k = 0; k < num_behaviors; ++k) {
				locomotion_behavior_t *locomotion_behavior = &behaviors[k];
				double p = (*locomotion_behavior)(zrc, id, point, front);
				potential += p;
			}
			potentials[i][j] = (float)potential;
			minp = min(minp, potential);
			maxp = max(maxp, potential);
		}
	}
	{
		double potential = 0;
		for (int k = 0; k < num_behaviors; ++k) {
			locomotion_behavior_t *locomotion_behavior = &behaviors[k];
			double p = (*locomotion_behavior)(zrc, id, physics->position, physics->front);
			potential += p;
		}
		current = (float)potential;
		minp = min(minp, potential);
		maxp = max(maxp, potential);
	}

	assert(minp != maxp);

	rl_obs_t obs;
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			double norm = (potentials[i][j] - minp) / (maxp - minp);
			obs.values[i][j] = (float)norm;
		}
	}
	{
		double norm = (current - minp) / (maxp - minp);
		obs.current = (float)norm;
	}
	ZRC_SEND(zrc, rl_obs, id, &obs);

	rl->reward = 0;
	const physics_t *pphysics = ZRC_GET_PREV(zrc, physics, id);
	if (physics && pphysics) {
		double potential = 0;
		double prev_potential = 0;
		for (int i = 0; i < num_behaviors; ++i) {
			locomotion_behavior_t *locomotion_behavior = &behaviors[i];
			prev_potential += (*locomotion_behavior)(zrc, id, pphysics->position, pphysics->front);
			potential += (*locomotion_behavior)(zrc, id, physics->position, physics->front);
		}
		double norm = (potential - minp) / (maxp - minp);
		double prev_norm = (prev_potential - minp) / (maxp - minp);
		float reward = (float)(norm - prev_norm);
		rl->reward += reward;
	}
	rl->total_reward += rl->reward;

	rl_act_t *action;
	ZRC_RECEIVE(zrc, rl_act, id, &rl->recv_rl_act, action, {
		flight_thrust_t flight_thrust;
		flight_thrust.thrust.x = step(action->thrust[0]);
		flight_thrust.thrust.y = step(action->thrust[1]);
		flight_thrust.turn = step(action->turn);
		flight_thrust.damp = action->damp < 0 ? 0.0f : SHIP_DAMPING;
		ZRC_SEND(zrc, flight_thrust, id, &flight_thrust);
	});
}

static unsigned has_cast[ABILITY_COUNT];

static ability_id_t ability_match(const rl_t *rl, float *v) {
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
