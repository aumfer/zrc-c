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
	life->health = min(life->max_health, life->health + (life->constitution/100) * TICK_RATE);
	life->mana = min(life->max_mana, life->mana + (life->willpower/100) * TICK_RATE);
	life->rage = max(0, life->rage - (life->serenity/100) * TICK_RATE);

	damage_t selfharm = {
		.from = id
	};

	damage_t *damage;
	ZRC_RECEIVE(zrc, damage, id, &life->damage_index, damage, {
		float taken = damage->health * (1 / max(1, life->strength/100));
		life->health = max(0, life->health - taken);

		float rage = taken * ((life->temper/100) * TICK_RATE);
		float outrage = (life->rage + rage) - life->max_rage;
		life->rage = min(life->max_rage, life->rage + rage);

		if (outrage > 0) {
			puts("ow");
			selfharm.health += outrage;
		}

		if (taken > 0) {
			damage_dealt_t damage_dealt = { 0 };
			damage_dealt.to = id;
			damage_dealt.ability = damage->ability;
			damage_dealt.health = taken;
			ZRC_SEND(zrc, damage_dealt, damage->from, &damage_dealt);
		}
	});
	if (selfharm.health > 0) {
		ZRC_SEND(zrc, damage, id, &selfharm);
	}
	if (life->health <= 0) {
		ZRC_DESPAWN_ALL(zrc, id);
	}
}
