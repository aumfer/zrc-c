#include <zrc_host.h>
#include <stdio.h>
#include <inttypes.h>

static void cast_tur_proj_attack(zrc_t *zrc, ability_id_t ability_id, id_t caster_id, const ability_target_t *target) {
	zrc_host_t *zrc_host = zrc->user;

	ability_t *ability = &zrc->ability[ability_id];
	const physics_t *physics = ZRC_GET(zrc, physics, caster_id);

	id_t proj_id = zrc_host_put(zrc_host, guid_create());
	float proj_speed = 250;

	cpVect front = physics->front;
	cpVect target_point = cpv(target->point[0], target->point[1]);
	cpVect dir = cpvnormalize(cpvsub(target_point, physics->position));
	physics_t proj_physics = {
		.collide_flags = ~0,
		.collide_mask = ~0,
		.response_mask = ~0,
		.radius = 0.1f,
		.position = cpvadd(physics->position, cpvmult(front, physics->radius)),
		.velocity = cpvmult(dir, proj_speed)
	};
	ZRC_SPAWN(zrc, physics, proj_id, &proj_physics);
	ZRC_SPAWN(zrc, ttl, proj_id, &(ttl_t) {
		.ttl = ability->range / proj_speed
	});
	ZRC_SPAWN(zrc, visual, proj_id, &(visual_t) {
		.color = color_random(255)
	});
	contact_damage_t contact_damage = {
		.damage = {
			.from = caster_id,
			.ability = ability_id,
			.health = 20
		},
		.flags = CONTACT_DAMAGE_DESPAWN_ON_HIT,
		.onhit_id = zrc_host_put(zrc_host, guid_create()),
		.visual = {
			.color = color_random(255),
			.size = { 10, 10 }
		},
		.ttl = {
			.ttl = 0.5f
		}
	};
	ZRC_SPAWN(zrc, contact_damage, proj_id, &contact_damage);
}
static void cast_blink(zrc_t *zrc, ability_id_t ability_id, id_t caster_id, const ability_target_t *target) {
	ability_t *ability = &zrc->ability[ability_id];
	physics_t *physics = ZRC_GET_WRITE(zrc, physics, caster_id);
	float dist = cpvdistsq(physics->position, cpv(target->point[0], target->point[1]));
	if (dist > ability->range*ability->range) {
		return;
	}
	physics->position.x = target->point[0];
	physics->position.y = target->point[1];
}
static void cast_fix_proj_attack(zrc_t *zrc, ability_id_t ability_id, id_t caster_id, const ability_target_t *target) {
	zrc_host_t *zrc_host = zrc->user;

	ability_t *ability = &zrc->ability[ability_id];
	const physics_t *physics = ZRC_GET(zrc, physics, caster_id);

	id_t proj_id = zrc_host_put(zrc_host, guid_create());
	float proj_speed = 500;

	cpVect front = physics->front;
	physics_t proj_physics = {
		.collide_flags = ~0,
		.collide_mask = ~0,
		.response_mask = ~0,
		.radius = 0.25f,
		.position = cpvadd(physics->position, cpvmult(front, physics->radius)),
		.velocity = cpvmult(front, proj_speed)
	};
	ZRC_SPAWN(zrc, physics, proj_id, &proj_physics);
	ZRC_SPAWN(zrc, ttl, proj_id, &(ttl_t) {
		.ttl = ability->range / proj_speed
	});
	ZRC_SPAWN(zrc, visual, proj_id, &(visual_t) {
		.color = color_random(255)
	});
	contact_damage_t contact_damage = {
		.damage = {
			.from = caster_id,
			.ability = ability_id,
			.health = 10
		},
		.flags = CONTACT_DAMAGE_DESPAWN_ON_HIT,
		.onhit_id = zrc_host_put(zrc_host, guid_create()),
		.visual = {
			.color = color_random(255),
			.size = { 10, 10 }
		},
		.ttl = {
			.ttl = 0.5f
		}
	};
	ZRC_SPAWN(zrc, contact_damage, proj_id, &contact_damage);
}
static void cast_target_nuke(zrc_t *zrc, ability_id_t ability_id, id_t caster_id, const ability_target_t *target) {
	zrc_host_t *zrc_host = zrc->user;

	ability_t *ability = &zrc->ability[ability_id];

	damage_t damage = {
		.from = caster_id,
		.health = 40
	};
	ZRC_SEND(zrc, damage, target->unit, &damage);

	const physics_t *target_physics = ZRC_GET(zrc, physics, target->unit);
	if (target_physics) {
		id_t hit_id = zrc_host_put(zrc_host, guid_create());
		ZRC_SPAWN(zrc, ttl, hit_id, &(ttl_t) {
			.ttl = 0.5f
		});
		ZRC_SPAWN(zrc, visual, hit_id, &(visual_t) {
			.color = color_random(255)
		});
		visual_t visual = {
			.color = color_random(255),
			.size = { 10, 10 },
			.position = { target_physics->position.x, target_physics->position.y }
		};
		ZRC_SPAWN(zrc, visual, hit_id, &visual);
	}
}

void zrc_host_startup(zrc_host_t *zrc_host, zrc_t *zrc) {
	//printf("zrc_host %zu\n", sizeof(zrc_host_t));

	zrc->user = zrc_host;

	zrc_host->entities = kh_init(ehash);
	kh_resize(ehash, zrc_host->entities, MAX_ENTITIES);

	zrc->ability[ABILITY_TUR_PROJ_ATTACK].cast = cast_tur_proj_attack;
	zrc->ability[ABILITY_BLINK].cast = cast_blink;
	zrc->ability[ABILITY_FIX_PROJ_ATTACK].cast = cast_fix_proj_attack;
	zrc->ability[ABILITY_TARGET_NUKE].cast = cast_target_nuke;

	demo_world_create(&zrc_host->demo_world, zrc_host, zrc);

	tf_brain_create(&zrc_host->brain, "C:\\GitHub\\aumfer\\zrc-learn\\log\\simple_save\\", RL_OBS_LENGTH, RL_ACT_LENGTH);
}
void zrc_host_shutdown(zrc_host_t *zrc_host) {
	tf_brain_delete(&zrc_host->brain);
	kh_destroy(ehash, zrc_host->entities);
}

void zrc_host_update(zrc_host_t *zrc_host, zrc_t *zrc) {
	++zrc_host->frame;

	for (int i = 0; i < MAX_ENTITIES; ++i) {
		registry_t *read = ZRC_GET_READ(zrc, registry, i);
		registry_t *prev = ZRC_GET_PREV(zrc, registry, i);
		if (*read == 0 && *prev != 0) {
			//printf("deleting %d\n", i);
			kh_del(ehash, zrc_host->entities, i);
		}
	}

	for (int i = 0; i < MAX_ENTITIES; ++i) {
		tf_brain_t *brain = &zrc_host->brain;
		tf_brain_update(zrc, i, brain);
	}
}

id_t zrc_host_put(zrc_host_t *zrc_host, guid_t guid) {
	int absent;
	khint_t k = kh_put(ehash, zrc_host->entities, guid, &absent);
	zrc_assert(absent > 0);
	return (id_t)k;
}
id_t zrc_host_del(zrc_host_t *zrc_host, guid_t guid) {
	khint_t k = kh_get(ehash, zrc_host->entities, guid);
	kh_del(ehash, zrc_host->entities, k);
	return (id_t)k;
}
id_t zrc_host_get(const zrc_host_t *zrc_host, guid_t guid) {
	khint_t k = kh_get(ehash, zrc_host->entities, guid);
	return k == kh_end(zrc_host->entities) ? ID_INVALID : (id_t)k;
}

void demo_world_create(demo_world_t *demo_world, zrc_host_t *zrc_host, zrc_t *zrc) {
	const float SMALL_SHIP = 2.5f;
	const float MEDIUM_SHIP = 5;
	const float LARGE_SHIP = 12.5;
	const float CAPITAL_SHIP = 50;
	for (int i = 0; i < NUM_TEST_ENTITIES; ++i) {
		id_t id = zrc_host_put(zrc_host, guid_create());
		demo_world->test_entities[i] = id;

		float rfaction = randf();
		team_t faction = (rfaction < (1.0 / 3.0)) ? TEAM_RADIANT : ((rfaction < 2.0 / 3.0) ? TEAM_DIRE : TEAM_OTHER);
		//faction = !i ? TEAM_RADIANT : TEAM_DIRE;
		ZRC_SPAWN(zrc, team, id, &faction);

		physics_t physics = {
			.type = CP_BODY_TYPE_DYNAMIC,
			.collide_flags = ~0,
			.collide_mask = ~0,
			.response_mask = ~0,
			.max_speed = 250,
			.max_spin = 2.5f,
			.damping = SHIP_DAMPING,
			//.radius = 0.5f,
			//.radius = !i ? SMALL_SHIP : randf() * 12 + 0.5f,
			.radius = MEDIUM_SHIP,
			.position = {.x = randfs() * SPAWN_HALF, .y = randfs() * SPAWN_HALF },
			.angle = randf() * 2 * CP_PI
		};
		ZRC_SPAWN(zrc, physics, id, &physics);
		//ZRC_SPAWN(zrc, physics_controller, id, &(physics_controller_t){0});
		ZRC_SPAWN(zrc, locomotion, id, &(locomotion_t){0});
		seek_t seek = {
			.weight = 1
		};
		ZRC_SPAWN(zrc, seek, id, &seek);
		align_t align = {
			.weight = 1
		};
		ZRC_SPAWN(zrc, align, id, &align);
		visual_t visual = {
			.color = color_random(255)
			//.color = faction == TEAM_RADIANT ? 0xff0000ff : (faction == TEAM_DIRE ? 0xff00ff00 : 0xffff0000)
		};
		ZRC_SPAWN(zrc, visual, id, &visual);
		flight_t flight = {
			// w/ controller
			//.max_thrust = 150,
			//.max_turn = 3,
			// w/ force
			.max_thrust = 150*5,
			.max_turn = 3*5,
			// instant
			//.thrust_control_rate = 1.0f / TICK_RATE,
			//.turn_control_rate = 1.0f / TICK_RATE,
			.thrust_control_rate = 2,
			.turn_control_rate = 4
		};
		ZRC_SPAWN(zrc, flight, id, &flight);
		life_t life = {
			.health = 75,
			.max_health = 100,
			.strength = 100,
			.constitution = 100,
			.mana = 25,
			.max_mana = 100,
			.focus = 100,
			.willpower = 100,
			.rage = 50,
			.max_rage = 100,
			.serenity = 100,
			.temper = 100
		};
		ZRC_SPAWN(zrc, life, id, &life);
		caster_t caster = {
			.abilities = {
				[0].ability = ABILITY_TUR_PROJ_ATTACK,
				[1].ability = ABILITY_FIX_PROJ_ATTACK,
				[2].ability = ABILITY_BLINK,
				[3].ability = ABILITY_TARGET_NUKE,
			}
		};
		ZRC_SPAWN(zrc, caster, id, &caster);
		ZRC_SPAWN(zrc, sense, id, &(sense_t) {
			.range = 500
		});

		rl_t rl = {
			.act = !!i
		};
		ZRC_SPAWN(zrc, rl, id, &rl);
		seek_to_t seek_to = {
			.point = {.x = randfs() * WORLD_HALF / 2,.y = randfs() * WORLD_HALF / 2 },
			.weight = 1
		};
		ZRC_SEND(zrc, seek_to, id, &seek_to);
		align_to_t align_to = {
			.angle = randfs() * CP_PI,
			.weight = 1 / 10000.0f
		};
		//ZRC_SEND(zrc, align_to, id, &align_to);

		id_t goal_id = zrc_host_put(zrc_host, guid_create());

		visual_t goal = {
			.position = {seek_to.point.x, seek_to.point.y},
			.angle = align_to.angle,
			.size = {25, 25},
			//.color = color_random(255)
			.color = visual.color
		};
		ZRC_SPAWN(zrc, visual, goal_id, &goal);

		//contact_damage_t contact_damage = {
		//	.damage = {
		//		.from = id,
		//		.health = 10
		//	},
		//	.onhit_id = ID_INVALID
		//};
		//ZRC_SPAWN(zrc, contact_damage, id, &contact_damage);
	}
}
void demo_world_delete(demo_world_t *demo_world) {

}