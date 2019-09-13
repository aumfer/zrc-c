#include <zrc.h>
#include <stdio.h>

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
	if (life->health > 1) {
		life->health -= 1 * TICK_RATE;
	}
	if (life->mana < life->max_mana) {
		life->mana += 1 * TICK_RATE;
	}
	ZRC_RECEIVE(zrc, damage, id, damage, {
		life->health = max(0, life->health - damage->health);
		});
	if (life->health <= 0) {
		ZRC_DESPAWN(zrc, life, id);
	}
}
