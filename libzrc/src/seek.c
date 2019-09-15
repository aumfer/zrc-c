#include <zrc.h>
#include <stdio.h>

void seek_startup(zrc_t *zrc) {
	printf("seek %zu\n", sizeof(zrc->seek));
}
void seek_shutdown(zrc_t *zrc) {

}
void seek_create(zrc_t *zrc, id_t id, seek_t *seek) {

}
void seek_delete(zrc_t *zrc, id_t id, seek_t *seek) {

}
static double seek_locomotion_behavior(zrc_t *zrc, id_t id, cpVect point) {
	seek_t *seek = ZRC_GET(zrc, seek, id);
	float distance = cpvdistsq(seek->point, point);
	return 1.0 / (1 + distance);
}
void seek_update(zrc_t *zrc, id_t id, seek_t *seek) {
	seek_to_t *seek_to;
	ZRC_RECEIVE(zrc, seek_to, id, &seek->num_seeks, seek_to, {
		seek->point = seek_to->point;
		});

	if (!seek->num_seeks) {
		// bail if we have never gotten a seek
		return;
	}

	ZRC_SEND(zrc, locomotion_behavior, id, seek_locomotion_behavior);
}