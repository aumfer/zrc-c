#include <zrc.h>
#include <stdio.h>

void flight_startup(zrc_t *zrc) {
	printf("flight %zu\n", sizeof(zrc->flight));
}
void flight_shutdown(zrc_t *zrc) {
}
void flight_create(zrc_t *zrc, id_t id, flight_t *flight) {

}
void flight_delete(zrc_t *zrc, id_t id, flight_t *flight) {

}
void flight_update(zrc_t *zrc, id_t id, flight_t *flight) {
	physics_t *physics = ZRC_GET(zrc, physics, id);
	assert(physics);

	cpVect thrust = cpv(flight->thrust[0], flight->thrust[1]);
	thrust.x = max(0, thrust.x);
	thrust = cpvclamp(thrust, 1);
	thrust = cpvrotate(thrust, cpvforangle(physics->angle));
	cpVect force = cpvmult(thrust, flight->max_thrust);

	float turn = flight->turn;
	turn = (float)cpfclamp(turn, -1, +1);
	float torque = turn * flight->max_turn;

	if (ZRC_HAS(zrc, physics_controller, id)) {
		physics_controller_velocity_t physics_controller_velocity = {
			.velocity = force,
			.angular_velocity = torque
		};
		ZRC_SEND(zrc, physics_controller_velocity, id, &physics_controller_velocity);
	}
	else {
		float damp = 2;

		cpVect force_damp = cpvmult(physics->velocity, -damp);
		float torque_damp = (float)(physics->angular_velocity * -damp);

		physics_force_t physics_force = {
			.force = cpvadd(force, force_damp),
			.torque = torque + torque_damp
		};
		ZRC_SEND(zrc, physics_force, id, &physics_force);
	}
}
