#include <zrc.h>
#include <stdio.h>

void flight_startup(zrc_t *zrc) {
	//printf("flight %zu\n", sizeof(zrc->flight));
}
void flight_shutdown(zrc_t *zrc) {
}
void flight_create(zrc_t *zrc, id_t id, flight_t *flight) {

}
void flight_delete(zrc_t *zrc, id_t id, flight_t *flight) {

}
void flight_update(zrc_t *zrc, id_t id, flight_t *flight) {
	physics_t *physics = ZRC_GET(zrc, physics, id);
	zrc_assert(physics);

	cpVect sum_force = cpvzero;
	float sum_torque = 0;
	int num_thrusts = 0;

	flight_thrust_t *flight_thrust;
	ZRC_RECEIVE(zrc, flight_thrust, id, &flight->thrust_index, flight_thrust, {
		cpVect thrust = cpv(flight_thrust->thrust[0], flight_thrust->thrust[1]);
		//thrust.x = max(0, thrust.x);
		thrust = cpvclamp(thrust, 1);
		thrust = cpvrotate(thrust, cpvforangle(physics->angle));
		cpVect force = cpvmult(thrust, flight->max_thrust);

		float turn = flight_thrust->turn;
		turn = (float)cpfclamp(turn, -1, +1);
		float torque = turn * flight->max_turn;

		sum_force = cpvadd(sum_force, force);
		sum_torque += torque;
		++num_thrusts;
	});

	cpVect force = cpvmult(sum_force, 1.0f / max(1, num_thrusts));
	float torque = sum_torque / max(1, num_thrusts);
	if (ZRC_HAS(zrc, physics_controller, id)) {
		if (num_thrusts) {
			physics_controller_velocity_t physics_controller_velocity = {
				.velocity = force,
				.angular_velocity = torque
			};
			ZRC_SEND(zrc, physics_controller_velocity, id, &physics_controller_velocity);
		}
	} else {
		float damp = 2;

		//cpVect force_damp = cpvmult(physics->velocity, -damp);
		//float torque_damp = (float)(physics->angular_velocity * -damp);
		//
		//cpVect apply_force = cpvadd(force, force_damp);
		//float apply_torque = torque + torque_damp;
		cpVect apply_force = force;
		float apply_torque = torque;

		if (!cpveql(apply_force, cpvzero) || apply_torque != 0) {
			physics_force_t physics_force = {
				.force = apply_force,
				.torque = apply_torque
			};
			ZRC_SEND(zrc, physics_force, id, &physics_force);
		}
	}
}
