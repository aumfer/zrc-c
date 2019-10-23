#include <zrc.h>
#include <stdio.h>

void locomotion_startup(zrc_t *zrc) {
	//printf("locomotion %zu\n", sizeof(zrc->locomotion));
}
void locomotion_shutdown(zrc_t *zrc) {

}
void locomotion_create(zrc_t *zrc, id_t id, locomotion_t *locomotion) {

}
void locomotion_delete(zrc_t *zrc, id_t id, locomotion_t *locomotion) {

}
static const float THRUSTS[] = {
	+0.1f,
	+0.25f,
	+0.5f,
	+1,
	//-1,
	//-0.5f,
	//-0.25f,
	//-0.1f,
	0
};
#define NUM_THRUSTS _countof(THRUSTS)
static const float TURNS[] = {
	-1,
	-0.5,
	-0.25,
	+0.5,
	+0.25,
	+1,
	0,
};
#define NUM_TURNS _countof(TURNS)

static_assert(NUM_LOCOMOTION_THRUSTS == NUM_THRUSTS, "incorrect NUM_LOCOMOTION_THRUSTS");
static_assert(NUM_LOCOMOTION_TURNS == NUM_TURNS, "incorrect NUM_LOCOMOTION_TURNS");

void locomotion_update(zrc_t *zrc, id_t id, locomotion_t *locomotion) {
	const flight_t *flight = ZRC_GET(zrc, flight, id);
	const physics_t *physics = ZRC_GET(zrc, physics, id);
	if (!flight || !physics) {
		return;
	}

	cpVect positions[NUM_THRUSTS][NUM_TURNS];
	float angles[NUM_THRUSTS][NUM_TURNS];

	locomotion->num_behaviors = 0;

	locomotion_behavior_t *locomotion_behavior;
	ZRC_RECEIVE(zrc, locomotion_behavior, id, &locomotion->recv_locomotion_behavior, locomotion_behavior, {
		locomotion->behaviors[locomotion->num_behaviors++] = *locomotion_behavior;
	});

	if (!locomotion->num_behaviors) {
		return;
	}
	return;
	if (ZRC_HAS(zrc, rl, id)) {
		return;
	}

	int num_behavior_samples = 0;
	double potentials[NUM_THRUSTS][NUM_TURNS];
	double minp = FLT_MAX;
	double maxp = -FLT_MAX;
	for (int i = 0; i < NUM_THRUSTS; ++i) {
		for (int j = 0; j < NUM_TURNS; ++j) {
			float turn = TURNS[j] * physics->max_spin * TICK_RATE;
			float angle = physics->angle + physics->angular_velocity * TICK_RATE;
			float test_angle = angle + turn;
			cpVect test_direction = cpvforangle(test_angle);
			float move = THRUSTS[i] * physics->max_speed * TICK_RATE;
			cpVect position = cpvadd(physics->position, cpvmult(physics->velocity, TICK_RATE));
			cpVect test_position = cpvadd(position, cpvrotate(cpv(move, 0), cpvforangle(test_angle)));

			positions[i][j] = test_position;
			angles[i][j] = test_angle;

			double potential = locomotion_potential(zrc, id, test_position, test_direction);
			potentials[i][j] = potential;
			minp = min(minp, potential);
			maxp = max(maxp, potential);
		}
	}

	assert(minp != maxp);

	int best_thrust = -1;
	int best_turn = -1;
	double best_potential = -FLT_MAX;
	for (int i = 0; i < NUM_THRUSTS; ++i) {
		for (int j = 0; j < NUM_TURNS; ++j) {
			double potential = potentials[i][j];
			if (potential >= best_potential) {
				best_potential = potential;
				best_thrust = i;
				best_turn = j;
			}
		}
	}

	if (THRUSTS[best_thrust] || TURNS[best_turn]) {
		flight_thrust_t flight_thrust = {
			.thrust = { THRUSTS[best_thrust], 0 },
			.turn = TURNS[best_turn],
			.damp = SHIP_DAMPING
		};
		ZRC_SEND(zrc, flight_thrust, id, &flight_thrust);
	}
}

double locomotion_potential(const zrc_t *zrc, id_t id, cpVect point, cpVect front) {
	double potential = 0;
	const locomotion_t *locomotion = ZRC_GET(zrc, locomotion, id);
	if (locomotion) {
		for (int i = 0; i < locomotion->num_behaviors; ++i) {
			const locomotion_behavior_t *locomotion_behavior = &locomotion->behaviors[i];
			double p = (*locomotion_behavior)(zrc, id, point, front);
			//potential += p;
			//potential *= p;
			potential = max(potential, p);
		}
	}
	return potential;
}