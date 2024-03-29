#include <zrc.h>
#include <stdio.h>

void physics_controller_startup(zrc_t *zrc) {
	//printf("physics_controller %zu\n", sizeof(zrc->physics_controller));
}
void physics_controller_shutdown(zrc_t *zrc) {
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		if (ZRC_HAS(zrc, physics_controller, i)) {
			physics_controller_t *physics_controller = ZRC_GET_WRITE(zrc, physics_controller, i);
			physics_controller_delete(zrc, i, physics_controller);
		}
	}
}
void physics_controller_create(zrc_t *zrc, id_t id, physics_controller_t *physics_controller) {
	// physics body might not exist yet, lazy init below
}
void physics_controller_delete(zrc_t *zrc, id_t id, physics_controller_t *physics_controller) {
	if (physics_controller->body) {
		cpSpaceRemoveConstraint(zrc->space, physics_controller->gear);
		cpSpaceRemoveConstraint(zrc->space, physics_controller->pivot);
		cpSpaceRemoveBody(zrc->space, physics_controller->body);

		cpConstraintFree(physics_controller->gear);
		cpConstraintFree(physics_controller->pivot);
		cpBodyFree(physics_controller->body);
	}
}
void physics_controller_update(zrc_t *zrc, id_t id, physics_controller_t *physics_controller) {
	if (!physics_controller->body) {
		const physics_t *physics = ZRC_GET(zrc, physics, id);
		zrc_assert(physics);

		physics_controller->body = cpBodyNew(1, 1);
		cpBodySetPosition(physics_controller->body, cpBodyGetPosition(physics->body));
		cpBodySetAngle(physics_controller->body, cpBodyGetAngle(physics->body));
		cpBodySetType(physics_controller->body, CP_BODY_TYPE_KINEMATIC);

		physics_controller->pivot = cpPivotJointNew2(physics_controller->body, physics->body, cpvzero, cpvzero);
		cpConstraintSetMaxBias(physics_controller->pivot, 0);
		cpConstraintSetMaxForce(physics_controller->pivot, physics->max_speed * physics->mass * 10);
		physics_controller->gear = cpGearJointNew(physics_controller->body, physics->body, 0, 1);
		cpConstraintSetMaxBias(physics_controller->gear, 0);
		cpConstraintSetMaxForce(physics_controller->gear, physics->max_spin * physics->moment * 10);
		//physics_controller->pivot = cpDampedSpringNew(physics_controller->body, physics->body, cpvzero, cpvzero, 0, 5, 3);
		//physics_controller->gear = cpDampedRotarySpringNew(physics_controller->body, physics->body, 0, 5, 3);

		cpBodySetUserData(physics_controller->body, (cpDataPointer)id);
		cpSpaceAddBody(zrc->space, physics_controller->body);
		cpSpaceAddConstraint(zrc->space, physics_controller->pivot);
		cpSpaceAddConstraint(zrc->space, physics_controller->gear);
	}
	physics_controller_velocity_t *physics_controller_velocity;
	cpVect velocity = cpvzero;
	float angular_velocity = 0;
	int count = 0;
	ZRC_RECEIVE(zrc, physics_controller_velocity, id, &physics_controller->recv_velocity, physics_controller_velocity, {
		velocity = cpvadd(velocity, physics_controller_velocity->velocity);
		angular_velocity += physics_controller_velocity->angular_velocity;
		++count;
	});
	cpBodySetVelocity(physics_controller->body, velocity);
	cpBodySetAngularVelocity(physics_controller->body, angular_velocity);
}
