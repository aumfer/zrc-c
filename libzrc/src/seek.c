#include <zrc.h>
#include <stdio.h>

void seek_startup(zrc_t *zrc) {
	//printf("seek %zu\n", sizeof(zrc->seek));
}
void seek_shutdown(zrc_t *zrc) {

}
void seek_create(zrc_t *zrc, id_t id, seek_t *seek) {

}
void seek_delete(zrc_t *zrc, id_t id, seek_t *seek) {

}
static double seek_locomotion_behavior(const zrc_t *zrc, id_t id, cpVect point, cpVect front) {
	const seek_t *seek = ZRC_GET(zrc, seek, id);
	float distance = cpvdistsq(seek->point, point);
	const physics_t *physics = ZRC_GET(zrc, physics, id);
	float min_distance = physics ? (physics->radius*physics->radius) : 1;
	cpVect offset = cpvsub(seek->point, physics->position);
	cpVect direction = cpvnormalize(offset);
	double potential = (1.0 / max(min_distance, distance)) + cpvdot(front, direction) * FLT_EPSILON;
	return potential;
}
void seek_update(zrc_t *zrc, id_t id, seek_t *seek) {
	seek_to_t *seek_to;
	ZRC_RECEIVE(zrc, seek_to, id, &seek->recv_seek_to, seek_to, {
		seek->point = seek_to->point;
	});

	if (!seek->recv_seek_to) {
		// bail if we have never gotten a seek
		return;
	}

	ZRC_SEND(zrc, locomotion_behavior, id, seek_locomotion_behavior);
}