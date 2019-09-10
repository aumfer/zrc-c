#include <zrc.h>

#include <tinycthread.h>
#include <stdio.h>
#include <stdatomic.h>
#include <time.h>

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

void visual_create(zrc_t *zrc, id_t id, visual_t *visual) {

}
void visual_delete(zrc_t *zrc, id_t id, visual_t *visual) {

}
void visual_update(zrc_t *zrc, id_t id, visual_t *visual) {
}

void flight_create(zrc_t *zrc, id_t id, flight_t *flight) {

}
void flight_delete(zrc_t *zrc, id_t id, flight_t *flight) {

}
void flight_update(zrc_t *zrc, id_t id, flight_t *flight) {
}

void motion_create(zrc_t *zrc, id_t id, motion_t *motion) {

}
void motion_delete(zrc_t *zrc, id_t id, motion_t *motion) {

}
void motion_update(zrc_t *zrc, id_t id, motion_t *motion) {
}

static zrc_t noalloc;

int main(int argc, char **argv) {
	//zrc_t *zrc = calloc(1, sizeof(zrc_t));
	zrc_t *zrc = &noalloc;
	printf("zrc %zu\n", sizeof(zrc_t));

	registry_startup(zrc);
	physics_startup(zrc);

	id_t id = 0;

	for (;;) {
		ZRC_UPDATE1(zrc, registry);
		

		if (zrc->frame == 1) {
			physics_t pe = {
				.radius = 0.5f
			};
			ZRC_SPAWN(zrc, physics, id, &pe);
		}
		motion_t mo = {
			.torque = 1
		};
		ZRC_SEND(zrc, motion, id, &mo);
		if (zrc->frame == 10) {
			ZRC_DESPAWN(zrc, physics, id);
		}

		ZRC_UPDATE1(zrc, flight);
		ZRC_UPDATE2(zrc, physics);
		ZRC_UPDATE1(zrc, visual);
		ZRC_UPDATE0(zrc, motion);

		++zrc->frame;
		thrd_yield();
	}
    return 0;

	physics_shutdown(zrc);
	registry_shutdown(zrc);
}