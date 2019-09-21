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

	cpVect thrust = cpvzero;
	float turn = 0;

	flight_thrust_t *flight_thrust;
	ZRC_RECEIVE(zrc, flight_thrust, id, &flight->thrust_index, flight_thrust, {
		thrust = cpvadd(thrust, flight_thrust->thrust);
		turn += flight_thrust->turn;
	});

	// noreverse
	thrust.x = max(0, thrust.x);
	// halfstrafe
	//thrust.y = thrust.y / 2;
	// nostrafe
	thrust.y = 0;
	thrust = cpvclamp(thrust, 1);
	thrust = cpvrotate(thrust, cpvforangle(physics->angle));

	turn = (float)cpfclamp(turn, -1, +1);

	flight->thrust = cpvlerp(flight->thrust, thrust, flight->thrust_control_rate*TICK_RATE);
	flight->turn = cpflerp(flight->turn, turn, flight->turn_control_rate*TICK_RATE);

	cpVect force = cpvmult(flight->thrust, flight->max_thrust);
	float torque = flight->turn * flight->max_turn;

	if (ZRC_HAS(zrc, physics_controller, id)) {
		/*if (!cpveql(force, cpvzero) || torque != 0)*/ {
			physics_controller_velocity_t physics_controller_velocity = {
				.velocity = force,
				.angular_velocity = torque
			};
			ZRC_SEND(zrc, physics_controller_velocity, id, &physics_controller_velocity);
		}
	} else {
		if (!cpveql(force, cpvzero) || torque != 0) {
			physics_force_t physics_force = {
				.force = force,
				.torque = torque
			};
			ZRC_SEND(zrc, physics_force, id, &physics_force);
		}
	}
}
