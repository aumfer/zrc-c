#include <zrc.h>

void align_startup(zrc_t *zrc) {

}
void align_shutdown(zrc_t *zrc) {

}
void align_create(zrc_t *zrc, id_t id, align_t *align) {

}
void align_delete(zrc_t *zrc, id_t id, align_t *align) {

}
static double align_locomotion_behavior(const zrc_t *zrc, id_t id, cpVect point, cpVect front) {
	const physics_t *physics = ZRC_GET(zrc, physics, id);
	const align_t *align = ZRC_GET(zrc, align, id);
	if (!align || !physics) return 0;
	double potential = 0;
	for (int i = 0; i < align->num_to; ++i) {
		const align_to_t *to = &align->to[i];
		double p = max(0, cpvdot(front, cpvforangle(to->angle)));
		p *= to->weight;
		potential += p;
	}
	potential *= align->weight;
	return potential;
}
void align_update(zrc_t *zrc, id_t id, align_t *align) {
	//align->num_to = 0;
	align_to_t *align_to;
	ZRC_RECEIVE(zrc, align_to, id, &align->recv_align_to, align_to, {
		align->to[align->num_to++] = *align_to;
	});

	if (!align->num_to) {
		return;
	}

	ZRC_SEND(zrc, locomotion_behavior, id, align_locomotion_behavior);
}