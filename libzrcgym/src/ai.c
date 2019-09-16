#include <ai.h>
#include <zmath.h>
#include <string.h>

void ai_startup(ai_t *ai) {
	for (int i = 0; i < ABILITY_COUNT; ++i) {
		ai->ability[i][i] = 1;
	}
}
void ai_shutdown(ai_t *ai) {

}
void ai_update(zrc_t *zrc, ai_t *ai) {
	memcpy(ai->state[ZRC_WRITE_FRAME(zrc)], ai->state[ZRC_READ_FRAME(zrc)], sizeof(ai->state[ZRC_WRITE_FRAME(zrc)]));
	memset(ai->reward[ZRC_WRITE_FRAME(zrc)], 0, sizeof(ai->reward[ZRC_WRITE_FRAME(zrc)]));

	for (int i = 0; i < MAX_ENTITIES; ++i) {
		float *state = ai->state[ZRC_WRITE_FRAME(zrc)][i];
		float *reward = &ai->reward[ZRC_WRITE_FRAME(zrc)][i];

		// gather state
		// calculate reward
	}
}

void ai_observe(const zrc_t *zrc, const ai_t *ai, id_t id, float *observation) {
	const physics_t *physics = ZRC_GET(zrc, physics, id);
	assert(physics);
	if (!physics) return;
	// todo read all sensed entities, sort?, observe ai->state, write all to numpy
}
static ability_id_t ability_match(const ai_t *ai, float *v) {
	float dots[ABILITY_COUNT];
	for (int i = 0; i < ABILITY_COUNT; ++i) {
		int n = ABILITY_COUNT;
		int incx = 1;
		int incy = 1;
		float d = sdot(&n, v, &incx, ai->ability[i], &incy);
		dots[i] = d;
	}
	int maxi = -1;
	float maxv;
	for (int i = 0; i < ABILITY_COUNT; ++i) {
		if (!i || dots[i] > maxv) {
			maxi = i;
			maxv = dots[i];
		}
	}
	return maxi;
}
void ai_act(zrc_t *zrc, const ai_t *ai, id_t id, float *action) {
	physics_t *physics = ZRC_GET(zrc, physics, id);
	assert(physics);
	if (!physics) return;

	int i = 0;
	physics_force_t physics_force = {
		.force = cpv(action[i++], action[i++]),
		.torque = action[i++]
	};
	ZRC_SEND(zrc, physics_force, id, &physics_force);
	ability_id_t ability_id = ability_match(ai, &action[i]);
	i += ABILITY_COUNT;
	if (ability_id > ABILITY_NONE) {
		ability_t *ability = &zrc->ability[ability_id];
		caster_t *caster = ZRC_GET(zrc, caster, id);
		if (caster) {
			caster_ability_id_t caster_ability_id = CASTER_ABILITY_INVALID;
			for (int j = 0; j < CASTER_MAX_ABLITIES; ++j) {
				if (caster->abilities[j].ability == ability_id) {
					caster_ability_id = j;
					break;
				}
			}
			if (caster_ability_id != CASTER_ABILITY_INVALID) {
				cpVect target_dir = cpv(action[i++], action[i++]);
				cpVect target_offset = cpvmult(target_dir, ability->range);
				cpVect target_point = cpvadd(physics->position, target_offset);
				cast_t cast = {
					.caster_ability = caster_ability_id,
					.cast_flags = CAST_WANTCAST,
					.target.point[0] = target_point.x,
					.target.point[1] = target_point.y
				};
				// todo unit target
				ZRC_SEND(zrc, cast, id, &cast);
			}
		}
	}
	assert(i == AI_ACTION_LENGTH);
}
