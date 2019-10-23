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
	const physics_t *physics = ZRC_GET(zrc, physics, id);
	double potential = 0;
	for (int i = 0; i < seek->num_to; ++i) {
		const seek_to_t *to = &seek->to[i];
		float distance = cpvdist(to->point, point);
		float cur_distance = cpvdist(to->point, physics->position);

		double min_distance = physics ? physics->radius : 1;
		//cpVect offset = cpvsub(to->point, physics->position);
		//cpVect direction = cpvnormalize(offset);
		//double p = cur_distance - distance;
		//double p = min_distance / max(min_distance, distance);
		double p = (cur_distance - distance) / max(min_distance, cur_distance);
		//p += cpvdot(front, direction) * FLT_EPSILON;
		p *= to->weight;
		potential += p;
	}
	
	potential *= seek->weight;
	return potential;
}
void seek_update(zrc_t *zrc, id_t id, seek_t *seek) {
	//seek->num_to = 0;
	seek_to_t *seek_to;
	ZRC_RECEIVE(zrc, seek_to, id, &seek->recv_seek_to, seek_to, {
		seek->to[seek->num_to++] = *seek_to;
	});

	if (!seek->num_to) {
		return;
	}

	ZRC_SEND(zrc, locomotion_behavior, id, seek_locomotion_behavior);
}