#include <zrc.h>
#include <stdio.h>

void physics_controller_startup(zrc_t *zrc) {
	printf("physics_controller %zu\n", sizeof(zrc->physics_controller));
}
void physics_controller_shutdown(zrc_t *zrc) {

}
void physics_controller_create(zrc_t *zrc, id_t id, physics_controller_t *physics_controller) {
	physics_t *physics = ZRC_GET(zrc, physics, id);
	assert(physics);

	physics_controller->body = cpBodyNew(1, 1);
	cpBodySetPosition(physics_controller->body, cpBodyGetPosition(physics->body));
	cpBodySetAngle(physics_controller->body, cpBodyGetAngle(physics->body));
	cpBodySetType(physics_controller->body, CP_BODY_TYPE_KINEMATIC);

	physics_controller->pivot = cpPivotJointNew2(physics_controller->body, physics->body, cpvzero, cpvzero);
	physics_controller->gear = cpGearJointNew(physics_controller->body, physics->body, 0, 1);
	//physics_controller->pivot = cpDampedSpringNew(physics_controller->body, physics->body, cpvzero, cpvzero, 0, 5, 3);
	//physics_controller->gear = cpDampedRotarySpringNew(physics_controller->body, physics->body, 0, 5, 3);

	cpBodySetUserData(physics_controller->body, (cpDataPointer)id);
	cpSpaceAddBody(zrc->space, physics_controller->body);
	cpSpaceAddConstraint(zrc->space, physics_controller->pivot);
	cpSpaceAddConstraint(zrc->space, physics_controller->gear);
}
void physics_controller_delete(zrc_t *zrc, id_t id, physics_controller_t *physics_controller) {
	cpSpaceRemoveConstraint(zrc->space, physics_controller->gear);
	cpSpaceRemoveConstraint(zrc->space, physics_controller->pivot);
	cpSpaceRemoveBody(zrc->space, physics_controller->body);

	cpConstraintFree(physics_controller->gear);
	cpConstraintFree(physics_controller->pivot);
	cpBodyFree(physics_controller->body);
}
void physics_controller_update(zrc_t *zrc, id_t id, physics_controller_t *physics_controller) {
	cpVect velocity = cpBodyGetVelocity(physics_controller->body);
	velocity = cpvlerp(velocity, cpv(physics_controller->velocity[0], physics_controller->velocity[1]), 2 * TICK_RATE);
	//velocity = cpv(physics_controller->velocity[0], physics_controller->velocity[1]);
	cpFloat angular_velocity = cpBodyGetAngularVelocity(physics_controller->body);
	angular_velocity = cpflerp(angular_velocity, physics_controller->angular_velocity, 2 * TICK_RATE);
	//angular_velocity = physics_controller->angular_velocity;
	cpBodySetVelocity(physics_controller->body, velocity);
	cpBodySetAngularVelocity(physics_controller->body, angular_velocity);
}