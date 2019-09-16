#include <zrc.h>
#include <stdio.h>

void ttl_startup(zrc_t *zrc) {
	printf("ttl %zu\n", sizeof(zrc->ttl));
}
void ttl_shutdown(zrc_t *zrc) {

}
void ttl_create(zrc_t *zrc, id_t id, ttl_t *ttl) {
}
void ttl_delete(zrc_t *zrc, id_t id, ttl_t *ttl) {
}
void ttl_update(zrc_t *zrc, id_t id, ttl_t *ttl) {
	ttl->alive += TICK_RATE;
	if (ttl->alive >= ttl->ttl) {
		ZRC_DESPAWN_ALL(zrc, id);
	}
}
