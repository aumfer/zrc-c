#include <zrc.h>
#include <stdio.h>

static cpBool physics_collision_begin(cpArbiter *arb, cpSpace *space, cpDataPointer userData);
static void physics_collision_separate(cpArbiter *arb, cpSpace *space, cpDataPointer userData);
static void physics_velocity_update(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt);

void physics_startup(zrc_t *zrc) {
	//printf("physics %zu\n", sizeof(zrc->physics));

	zrc->space = cpSpaceNew();
	//const float LOAD_FACTOR = 0.1f;
	//cpSpaceUseSpatialHash(space, MAP_SCALE, PHYSICS_MAX_ENTITIES/LOAD_FACTOR);
	cpSpaceSetUserData(zrc->space, zrc);

	zrc->collision_handler = cpSpaceAddDefaultCollisionHandler(zrc->space);
	zrc->collision_handler->beginFunc = physics_collision_begin;
	zrc->collision_handler->separateFunc = physics_collision_separate;
}
static physics_deletebody(cpBody *body, void *data) {
	cpSpaceRemoveBody(cpBodyGetSpace(body), body);
	cpBodyFree(body);
}
static physics_deleteshape(cpShape *shape, void *data) {
	cpSpaceRemoveShape(cpShapeGetSpace(shape), shape);
	cpShapeFree(shape);
}
void physics_shutdown(zrc_t *zrc) {
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		physics_t *physics = ZRC_GET(zrc, physics, i);
		if (physics) {
			physics_delete(zrc, i, physics);
		}
	}
	//cpSpaceEachBody(zrc->space, physics_deletebody, 0);
	//cpSpaceEachShape(zrc->space, physics_deleteshape, 0);
	cpSpaceFree(zrc->space);
}

void physics_create(zrc_t *zrc, id_t id, physics_t *physics) {
	cpFloat mass = physics->radius * 2 * CP_PI;
	//cpFloat mass = 1;
	cpFloat moment = cpMomentForCircle(mass, 0, physics->radius*2, cpvzero);
	//cpFloat moment = 1;
	physics->body = cpBodyNew(mass, moment);
	cpBodySetType(physics->body, physics->type);
	cpBodySetVelocityUpdateFunc(physics->body, physics_velocity_update);
	// have to call this for static objs
	physics_begin(zrc, id, physics);

	cpVect offset = cpvzero;
	physics->shape = cpCircleShapeNew(physics->body, physics->radius, offset);
	cpShapeFilter filter;
	filter.group = id;
	filter.categories = physics->collide_flags;
	filter.mask = physics->collide_mask;
	cpShapeSetFilter(physics->shape, filter);

	cpBodySetUserData(physics->body, (cpDataPointer)id);
	cpShapeSetUserData(physics->shape, (cpDataPointer)id);
	cpSpaceAddBody(zrc->space, physics->body);
	cpSpaceAddShape(zrc->space, physics->shape);
}
void physics_delete(zrc_t *zrc, id_t id, physics_t *physics) {
	cpSpaceRemoveShape(zrc->space, physics->shape);
	cpSpaceRemoveBody(zrc->space, physics->body);
	cpShapeFree(physics->shape);
	cpBodyFree(physics->body);
}
void physics_begin(zrc_t *zrc, id_t id, physics_t *physics) {
	cpBodySetPosition(physics->body, physics->position);
	cpBodySetAngle(physics->body, physics->angle);
	cpBodySetVelocity(physics->body, physics->velocity);
	cpBodySetAngularVelocity(physics->body, physics->angular_velocity);

	cpVect force = cpvzero;
	float torque = 0;
	int num_forces = 0;
	physics_force_t *physics_force;
	ZRC_RECEIVE(zrc, physics_force, id, &physics->force_index, physics_force, {
		force = cpvadd(force, physics_force->force);
		torque += physics_force->torque;
		++num_forces;
	});
	if (num_forces > 1) {
		puts("hi");
	}
	cpBodySetForce(physics->body, force);
	cpBodySetTorque(physics->body, torque);
}
void physics_update(zrc_t *zrc) {
	cpSpaceStep(zrc->space, TICK_RATE);
}
void physics_end(zrc_t *zrc, id_t id, physics_t *physics) {
	cpVect position = cpBodyGetPosition(physics->body);
	physics->position = position;

	cpFloat angle = cpBodyGetAngle(physics->body);
	physics->angle = (float)angle;

	cpVect velocity = cpBodyGetVelocity(physics->body);
	physics->velocity = velocity;

	cpFloat angular_velocity = cpBodyGetAngularVelocity(physics->body);
	physics->angular_velocity = (float)angular_velocity;

	cpVect force = cpBodyGetForce(physics->body);
	physics->force = force;

	cpFloat torque = cpBodyGetTorque(physics->body);
	physics->torque = (float)torque;
}

id_t physics_query_ray(const zrc_t *zrc, cpVect start, cpVect end, float radius) {
	cpSegmentQueryInfo info;
	cpShape *shape = cpSpaceSegmentQueryFirst(zrc->space, start, end, radius, CP_SHAPE_FILTER_ALL, &info);
	if (shape) {
		id_t id = (id_t)cpShapeGetUserData(shape);
		return id;
	}
	return ID_INVALID;
}

id_t physics_query_point(const zrc_t *zrc, cpVect point, float radius) {
	cpPointQueryInfo info;
	cpShape *shape = cpSpacePointQueryNearest(zrc->space, point, radius, CP_SHAPE_FILTER_ALL, &info);
	if (shape) {
		id_t id = (id_t)cpShapeGetUserData(shape);
		return id;
	}
	return ID_INVALID;
}

static cpBool physics_collision_begin(cpArbiter *arb, cpSpace *space, cpDataPointer userData) {
	cpShape *s1, *s2;
	cpArbiterGetShapes(arb, &s1, &s2);

	id_t e1 = (id_t)cpShapeGetUserData(s1);
	id_t e2 = (id_t)cpShapeGetUserData(s2);

	zrc_t *zrc = cpSpaceGetUserData(space);
	physics_t *physics1 = ZRC_GET(zrc, physics, e1);
	physics_t *physics2 = ZRC_GET(zrc, physics, e2);

	cpBool respond;
	if (physics1 && physics2) {
		respond = physics1->response_mask & physics2->response_mask;
	} else {
		respond = cpFalse;
	}

	return respond;
}
static void physics_collision_separate(cpArbiter *arb, cpSpace *space, cpDataPointer userData) {
}

static void physics_velocity_update(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt) {
	id_t id = (id_t)cpBodyGetUserData(body);

	cpSpace *space = cpBodyGetSpace(body);
	zrc_t *zrc = cpSpaceGetUserData(space);
	physics_t *physics = ZRC_GET(zrc, physics, id);

	cpBodyUpdateVelocity(body, gravity, 1 - physics->damping, dt);

	if (physics) {
		if (physics->max_speed) {
			cpVect v = cpBodyGetVelocity(body);
			v = cpvclamp(v, physics->max_speed);
			cpBodySetVelocity(body, v);
		}

		if (physics->max_spin) {
			cpFloat angular_velocity = cpBodyGetAngularVelocity(body);
			cpFloat spin = cpfabs(angular_velocity);
			cpFloat spin_percent = spin / physics->max_spin;
			if (spin_percent > 1) {
				angular_velocity *= (1 / spin_percent);
				cpBodySetAngularVelocity(body, angular_velocity);
			}
		}
	}
}
