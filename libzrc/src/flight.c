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
	//thrust.y = max(0, thrust.y);
	thrust = cpvclamp(thrust, 1);
	thrust = cpvrotate(thrust, cpvforangle(physics->angle));
	cpVect force = cpvmult(thrust, flight->max_thrust);

	float turn = flight->turn;
	turn = (float)cpfclamp(turn, -1, +1);
	float torque = turn * flight->max_turn;

	if (ZRC_HAS(zrc, physics_controller, id)) {
		physics_controller_t *controller = ZRC_GET_WRITE(zrc, physics_controller, id);
		controller->velocity[0] = (float)force.x;
		controller->velocity[1] = (float)force.y;
		controller->angular_velocity = (float)torque;
	}
	else {
		physics_t *set = ZRC_GET_WRITE(zrc, physics, id);

		float damp = 2;

		cpVect force_damp = cpvmult(cpv(physics->velocity[0], physics->velocity[1]), -damp);
		float torque_damp = (float)(physics->angular_velocity * -damp);

		set->force[0] = (float)(force.x + force_damp.x);
		set->force[1] = (float)(force.y + force_damp.y);
		set->torque = torque + torque_damp;
	}
}
