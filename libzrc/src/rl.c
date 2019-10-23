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
	const locomotion_t *locomotion = ZRC_GET(zrc, locomotion, id);
	if (!physics || !locomotion) return;

	if (!locomotion->num_behaviors) return;

	double potentials[RL_OBS_NUM_MOVES][RL_OBS_NUM_TURNS];
	double minp = FLT_MAX;
	double maxp = -FLT_MAX;
	for (int i = 0; i < RL_OBS_NUM_MOVES; ++i) {
		cpVect move_dir;
		if (i < RL_OBS_NUM_MOVES-1) {
			float move_angle = snorm((float)i / RL_OBS_NUM_MOVES-1) * CP_PI;
			move_angle += physics->angle;
			move_dir = cpvforangle(move_angle);
			//move_dir = cpvrotate(move_dir, physics->front);
		} else {
			move_dir = cpvzero;
		}
		//cpVect move = cpvmult(move_dir, physics->max_speed * TICK_RATE);
		cpVect move = cpvmult(move_dir, 0.1f);
		cpVect point = cpvadd(physics->position, move);
		for (int j = 0; j < RL_OBS_NUM_TURNS; ++j) {
			
			float turn_angle = snorm((float)j / RL_OBS_NUM_TURNS) * CP_PI;
			turn_angle += physics->angle;
			cpVect front = cpvforangle(turn_angle);
			//front = cpvrotate(front, physics->front);

			double potential = locomotion_potential(zrc, id, point, front);
			potentials[i][j] = (float)potential;
			minp = min(minp, potential);
			maxp = max(maxp, potential);
		}
	}

	//assert(minp != maxp);

	rl_obs_t obs;
	for (int i = 0; i < RL_OBS_NUM_MOVES; ++i) {
		for (int j = 0; j < RL_OBS_NUM_TURNS; ++j) {
			double norm = znorm(potentials[i][j], minp, maxp);
			obs.values[i][j] = (float)norm;
		}
	}
	ZRC_SEND(zrc, rl_obs, id, &obs);

	rl->reward = 0;
	const physics_t *pphysics = ZRC_GET_PREV(zrc, physics, id);
	if (physics && pphysics) {
		double potential = locomotion_potential(zrc, id, physics->position, physics->front);
		double prev_potential = locomotion_potential(zrc, id, pphysics->position, pphysics->front);
		double norm = znorm(potential, minp, maxp);
		double prev_norm = znorm(prev_potential, minp, maxp);
		float reward = (float)(norm - prev_norm);
		//float reward = (float)(norm * TICK_RATE);
		float speed = cpvlength(physics->velocity) + physics->angular_velocity;
		reward *= speed;
		reward = max(0, reward);
		rl->reward += reward;
	}
	rl->total_reward += rl->reward;

	if (rl->act) {
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
