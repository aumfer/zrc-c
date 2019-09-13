#include <zrc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define SOKOL_IMPL
#include <sokol_time.h>
#include <stdarg.h>
#define HANDMADE_MATH_IMPLEMENTATION
#include <HandmadeMath.h>

registry_t zrc_components(int count, ...) {
	registry_t components = 0;
	va_list argp;
	va_start(argp, count);
	for (int i = 0; i < count; ++i) {
		registry_t next = va_arg(argp, registry_t);
		components |= next;
	}
	va_end(argp);
	return components;
}

void zrc_startup(zrc_t *zrc) {
	printf("zrc %zu\n", sizeof(zrc_t));

	stm_setup();
	timer_create(&zrc->timer);

	registry_startup(zrc);
	physics_startup(zrc);
	visual_startup(zrc);
	flight_startup(zrc);
	life_startup(zrc);
}
void zrc_shutdown(zrc_t *zrc) {
	life_shutdown(zrc);
	flight_shutdown(zrc);
	visual_shutdown(zrc);
	physics_shutdown(zrc);
	registry_shutdown(zrc);
}

void zrc_tick(zrc_t *zrc) {
	timer_update(&zrc->timer);

	double dts = stm_sec(zrc->timer.dt);
	moving_average_update(&zrc->fps, (float)dts);

	zrc->accumulator += dts;
	int frames = 0;
	while (zrc->accumulator >= TICK_RATE) {
		zrc->accumulator -= TICK_RATE;
		++frames;

		ZRC_UPDATE0(zrc, registry);
		ZRC_UPDATE1(zrc, flight);
		ZRC_UPDATE2(zrc, physics);
		ZRC_UPDATE1(zrc, physics_controller);
		ZRC_CLEAR(zrc, damage);
		ZRC_UPDATE1(zrc, life);
		ZRC_UPDATE1(zrc, visual);
	}

	if (frames > 1) {
		printf("stall %d frames\n", frames);
	}
}

void registry_startup(zrc_t *zrc) {
	printf("registry %zu\n", sizeof(zrc->registry));
}
void registry_shutdown(zrc_t *zrc) {

}
void registry_create(zrc_t *zrc, id_t id, registry_t *registry) {

}
void registry_delete(zrc_t *zrc, id_t id, registry_t *registry) {

}
void registry_update(zrc_t *zrc, id_t id, registry_t *registry) {
}

void visual_startup(zrc_t *zrc) {
	printf("visual %zu\n", sizeof(zrc->visual));
}
void visual_shutdown(zrc_t *zrc) {
}
void visual_create(zrc_t *zrc, id_t id, visual_t *visual) {

}
void visual_delete(zrc_t *zrc, id_t id, visual_t *visual) {

}
void visual_update(zrc_t *zrc, id_t id, visual_t *visual) {
}

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
	} else {
		physics_t *set = ZRC_GET_WRITE(zrc, physics, id);

		float damp = 2;

		cpVect force_damp = cpvmult(cpv(physics->velocity[0], physics->velocity[1]), -damp);
		float torque_damp = (float)(physics->angular_velocity * -damp);

		set->force[0] = (float)(force.x + force_damp.x);
		set->force[1] = (float)(force.y + force_damp.y);
		set->torque = torque + torque_damp;
	}
}

void life_startup(zrc_t *zrc) {
	printf("life %zu\n", sizeof(zrc->life));
}
void life_shutdown(zrc_t *zrc) {
}
void life_create(zrc_t *zrc, id_t id, life_t *life) {

}
void life_delete(zrc_t *zrc, id_t id, life_t *life) {

}
void life_update(zrc_t *zrc, id_t id, life_t *life) {
	damage_t *damage;
	life->health -= 1 * TICK_RATE;
	life->mana += 1 * TICK_RATE;
	ZRC_RECEIVE(zrc, damage, id, damage, {
		life->health = max(0, life->health - damage->health);
	});
	if (life->health <= 0) {
		ZRC_DESPAWN(zrc, life, id);
	}
}

void physics_controller_startup(zrc_t *zrc) {

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