#include <zrc.h>
#include <stdio.h>
#include <string.h>

void sense_startup(zrc_t *zrc) {
	//printf("sense %zu\n", sizeof(zrc->sense));
}
void sense_shutdown(zrc_t *zrc) {

}
void sense_create(zrc_t *zrc, id_t id, sense_t *sense) {
	memset(sense->entities, 0xffffffff, sizeof(sense->entities));
}
void sense_delete(zrc_t *zrc, id_t id, sense_t *sense) {

}
static void sense_bb_query(cpShape *shape, void *data) {
	sense_t *sense = data;
	id_t id = (id_t)cpShapeGetUserData(shape);
	
	if (sense->num_entities < SENSE_MAX_ENTITIES) {
		sense->entities[sense->num_entities++] = id;
	} else {
		puts("sense overflow");
	}
}
void sense_update(zrc_t *zrc, id_t id, sense_t *sense) {
	const physics_t *physics = ZRC_GET(zrc, physics, id);
	cpBB bounds = {
		.l = physics->position.x - physics->radius - sense->range,
		.r = physics->position.x + physics->radius + sense->range,
		.b = physics->position.y - physics->radius - sense->range,
		.t = physics->position.y + physics->radius + sense->range
	};
	sense->num_entities = 0;
	cpSpaceBBQuery(zrc->space, bounds, CP_SHAPE_FILTER_ALL, sense_bb_query, sense);
}
