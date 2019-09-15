#include <zrc.h>
#include <stdio.h>

void caster_startup(zrc_t *zrc) {
	printf("caster %zu\n", sizeof(zrc->caster));
}
void caster_shutdown(zrc_t *zrc) {
}
void caster_create(zrc_t *zrc, id_t id, caster_t *caster) {

}
void caster_delete(zrc_t *zrc, id_t id, caster_t *caster) {

}
void caster_update(zrc_t *zrc, id_t id, caster_t *caster) {
	cast_t *cast;
	ZRC_RECEIVE(zrc, cast, id, &caster->cast_index, cast, {
		caster_ability_t *caster_ability = &caster->abilities[cast->caster_ability];
		if ((cast->cast_flags & CAST_WANTCAST) == CAST_WANTCAST) {
			caster_ability->cast_flags |= CAST_WANTCAST;
		}
 else {
  caster_ability->cast_flags &= ~CAST_WANTCAST;
}
caster_ability->target = cast->target;
		});
	for (int i = 0; i < CASTER_MAX_ABLITIES; ++i) {
		caster_ability_t *caster_ability = &caster->abilities[i];
		const ability_t *ability = &zrc->ability[caster_ability->ability];
		if ((caster_ability->cast_flags & CAST_ISCAST) == CAST_ISCAST) {
			if (ability->cast) {
				ability->cast(zrc, ability, id, &caster_ability->target);
			}
			caster_ability->uptime += TICK_RATE;
			if (caster_ability->uptime >= ability->channel) {
				caster_ability->cast_flags &= ~CAST_ISCAST;
				caster_ability->uptime -= ability->channel;
				caster_ability->downtime += caster_ability->uptime;
				caster_ability->uptime = 0;
			}
		}
		else {
			caster_ability->downtime += TICK_RATE;
			if ((caster_ability->cast_flags & CAST_WANTCAST) == CAST_WANTCAST) {
				if (caster_ability->downtime >= ability->cooldown) {
					caster_ability->downtime -= ability->cooldown;
					caster_ability->uptime += caster_ability->downtime;
					caster_ability->downtime = 0;
					if (ability->cast) {
						ability->cast(zrc, ability, id, &caster_ability->target);
					}
				}
			}
		}
	}
}