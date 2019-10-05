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
	return cpvdot(front, cpvforangle(align->angle));
}
void align_update(zrc_t *zrc, id_t id, align_t *align) {
	align_to_t *align_to;
	ZRC_RECEIVE(zrc, align_to, id, &align->recv_align_to, align_to, {
		align->angle = align_to->angle;
	});

	if (!align->recv_align_to) {
		// bail if we have never gotten an align
		return;
	}

	ZRC_SEND(zrc, locomotion_behavior, id, align_locomotion_behavior);
}