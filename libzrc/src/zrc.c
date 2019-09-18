#include <zrc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define SOKOL_IMPL
#include <sokol_time.h>
#include <stdarg.h>

registry_t zrc_components(int count, ...) {
	registry_t components = 0;
	va_list argp;
	va_start(argp, count);
	for (int i = 0; i < count; ++i) {
		registry_t next = va_arg(argp, registry_t);
		components |= next;
	}
	va_end(argp);
	return components;
}

void zrc_startup(zrc_t *zrc) {
	//printf("zrc %zu\n", sizeof(zrc_t));

	registry_startup(zrc);
	flight_startup(zrc);
	physics_startup(zrc);
	physics_controller_startup(zrc);
	visual_startup(zrc);
	life_startup(zrc);
	caster_startup(zrc);
	ttl_startup(zrc);
	contact_damage_startup(zrc);
	locomotion_startup(zrc);
	seek_startup(zrc);
	sense_startup(zrc);
	relate_startup(zrc);
	ai_startup(zrc);

	zrc->ability[ABILITY_TUR_PROJ_ATTACK] = (ability_t) {
		.target_flags = ABILITY_TARGET_POINT,
		.range = 250,
		.cooldown = 2.0f / 3.0f / 2.0f,
		.channel = 1.0f / 3.0f / 2.0f,
		.mana = 10
	};
	zrc->ability[ABILITY_BLINK] = (ability_t) {
		.target_flags = ABILITY_TARGET_POINT,
		.range = 250,
		.cooldown = 7,
		.channel = 3,
		.mana = 40
	};
	zrc->ability[ABILITY_FIX_PROJ_ATTACK] = (ability_t) {
		.target_flags = ABILITY_TARGET_POINT,
		.range = 250,
		.cooldown = 2.0f / 3.0f,
		.channel = 1.0f / 3.0f,
		.mana = 20
	};
	zrc->ability[ABILITY_TARGET_NUKE] = (ability_t) {
		.target_flags = ABILITY_TARGET_UNIT,
		.range = 250,
		.cooldown = 3.0f,
		.channel = 2.0f,
		.mana = 30
	};

	timer_create(&zrc->timer);
}
void zrc_shutdown(zrc_t *zrc) {
	ai_shutdown(zrc);
	relate_shutdown(zrc);
	sense_shutdown(zrc);
	seek_shutdown(zrc);
	locomotion_shutdown(zrc);
	contact_damage_shutdown(zrc);
	ttl_shutdown(zrc);
	caster_shutdown(zrc);
	life_shutdown(zrc);
	visual_shutdown(zrc);
	physics_controller_shutdown(zrc);
	physics_shutdown(zrc);
	flight_shutdown(zrc);
	registry_shutdown(zrc);
}

void zrc_tick(zrc_t *zrc) {
	timer_update(&zrc->timer);

	double dts = stm_sec(zrc->timer.dt);
	moving_average_update(&zrc->tick_fps, (float)dts);

	zrc->accumulator += dts;
	int frames = 0;
	while (zrc->accumulator >= TICK_RATE) {
		zrc->accumulator -= TICK_RATE;
		++frames;
		
		zrc_update(zrc);
	}

	if (frames > 1) {
		//printf("stall %d frames\n", frames-1);
	}
}

void zrc_update(zrc_t *zrc) {
	++zrc->frame;

	//int i = 0;
	//printf("%u %d", zrc->frame, i++);
	ZRC_UPDATE0(zrc, registry);
	//printf(".%d", i++);
	ZRC_UPDATE1(zrc, flight);
	//printf(".%d", i++);
	ZRC_UPDATE1(zrc, physics_controller);
	//printf(".%d", i++);
	ZRC_UPDATE2(zrc, physics);
	//printf(".%d", i++);
	ZRC_UPDATE1(zrc, life);
	//printf(".%d", i++);
	ZRC_UPDATE0(zrc, visual);
	//printf(".%d", i++);
	ZRC_UPDATE1(zrc, caster);
	//printf(".%d", i++);
	ZRC_UPDATE1(zrc, ttl);
	//printf(".%d", i++);
	ZRC_UPDATE1(zrc, contact_damage);
	//printf(".%d", i++);
	ZRC_UPDATE1(zrc, locomotion);
	//printf(".%d", i++);
	ZRC_UPDATE1(zrc, seek);
	//printf(".%d", i++);
	ZRC_UPDATE1(zrc, sense);
	//printf(".%d", i++);
	ZRC_UPDATE0(zrc, relate);
	//printf(".%d", i++);
	ZRC_UPDATE1(zrc, ai);
	//puts(".done");

	uint64_t update_ticks = 0;
	for (int i = 0; i < zrc_component_count; ++i) {
		update_ticks += zrc->times[i];
	}
	float update_sec = (float)stm_sec(update_ticks);
	moving_average_update(&zrc->update_fps, update_sec);
}

void registry_startup(zrc_t *zrc) {
	//printf("registry %zu\n", sizeof(zrc->registry));
}
void registry_shutdown(zrc_t *zrc) {

}
void registry_update(zrc_t *zrc) {

}

void visual_startup(zrc_t *zrc) {
	//printf("visual %zu\n", sizeof(zrc->visual));
}
void visual_shutdown(zrc_t *zrc) {
}
void visual_update(zrc_t *zrc) {

}

void relate_startup(zrc_t *zrc) {
	//printf("relate %zu\n", sizeof(zrc->relate));
}
void relate_shutdown(zrc_t *zrc) {

}
static int relate_to_min(relate_t *relate) {
	int min = -1;
	float minv;
	for (int i = 0; i < relate->num_relates; ++i) {
		float absv = fabsf(relate->to[i].value);
		if (!i || absv < minv) {
			min = i;
			minv = absv;
		}
	}
	return min;
}
static void relate_change_receive(zrc_t *zrc, relate_t *relate, id_t to, float value) {
	int found = 0;
	for (int i = 0; i < relate->num_relates; ++i) {
		if (relate->to[i].id == to) {
			relate->to[i].value += value;
			found = 1;
			break;
		}
	}
	if (!found) {
		if (relate->num_relates < RELATE_MAX_TO) {
			relate->to[relate->num_relates++] = (relate_to_t){
				.id = to,
				.value = value
			};
		} else {
			int i = relate_to_min(relate);
			relate->to[i] = (relate_to_t) {
				.id = to,
				.value = value
			};
		}
	}
}
void relate_update(zrc_t *zrc) {
	for (int i = 0; i < zrc->num_relate_changes; ++i) {
		relationship_t *relate_change = &zrc->relate_change[i];
		relate_t *a = ZRC_GET_WRITE(zrc, relate, relate_change->from);
		relate_t *b = ZRC_GET_WRITE(zrc, relate, relate_change->to);
		relate_change_receive(zrc, a, relate_change->to, relate_change->amount);
		relate_change_receive(zrc, b, relate_change->from, relate_change->amount);
	}
	zrc->num_relate_changes = 0;
}

static int bfs(const zrc_t *zrc, id_t from, id_t to, id_t parent[MAX_ENTITIES]) {
	uint8_t visited[MAX_ENTITIES / 8] = { 0 };
	id_t q[MAX_ENTITIES];
	unsigned q_head = 0;
	unsigned q_tail = 0;
	q[q_head++&MASK_ENTITIES] = from;
	visited[from / 8] |= (1 << (from & 7));
	parent[from] = ID_INVALID;
	while (q_head != q_tail) {
		id_t u = q[q_tail++&MASK_ENTITIES];
		const relate_t *relate = &zrc->relate_query[u];
		if (!relate) continue;
		for (int i = 0; i < relate->num_relates; ++i) {
			id_t v = relate->to[i].id;
			if (!(visited[v / 8] & (1 << (v & 7))) && relate->to[i].value) {
				q[q_head++&MASK_ENTITIES] = v;
				parent[v] = u;
				visited[v / 8] |= (1 << (v & 7));
			}
		}
	}
	return visited[to / 8] & (1 << (to & 7));
}
static float ff_eka(zrc_t *zrc, id_t from, id_t to) {
	memcpy(zrc->relate_query, zrc->relate[zrc->frame&MASK_FRAMES], sizeof(zrc->relate[zrc->frame&MASK_FRAMES]));
	id_t parent[MAX_ENTITIES];
	memset(parent, 0xffffffff, sizeof(parent));
	float maxflow = 0;
	int num_paths = 0;
	while (bfs(zrc, from, to, parent)) {
		++num_paths;
		float pathflow = FLT_MAX;
		id_t v = to;
		while (v != from) {
			id_t u = parent[v];

			float capacity = 0;
			for (int i = 0; i < zrc->relate_query[u].num_relates; ++i) {
				if (zrc->relate_query[u].to[i].id == v) {
					capacity = 1 / fabsf(zrc->relate_query[u].to[i].value);
				}
			}

			pathflow = min(pathflow, capacity);

			v = u;
		}

		v = to;
		while (v != from) {
			id_t u = parent[v];

			for (int i = 0; i < zrc->relate_query[u].num_relates; ++i) {
				if (zrc->relate_query[u].to[i].id == v) {
					int pos = zrc->relate_query[u].to[i].value >= 0;
					zrc->relate_query[u].to[i].value -= 1.0f / (pos ? pathflow : -pathflow);
				}
			}
			for (int i = 0; i < zrc->relate_query[v].num_relates; ++i) {
				if (zrc->relate_query[v].to[i].id == u) {
					int pos = zrc->relate_query[u].to[i].value >= 0;
					zrc->relate_query[v].to[i].value += 1.0f / (pos ? pathflow : -pathflow);
				}
			}

			v = u;
		}
		maxflow += 1.0f/pathflow;
	}
	return num_paths ? maxflow : NAN;
}
float relate_to_query(zrc_t *zrc, id_t a, id_t b) {
	float flow1 = ff_eka(zrc, a, b);
	//float flow2 = ff_eka(zrc, b, a);
	return flow1;
}
