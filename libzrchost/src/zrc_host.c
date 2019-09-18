#include <zrc_host.h>
#include <stdio.h>

static void cast_tur_proj_attack(zrc_t *zrc, ability_id_t ability_id, id_t caster_id, const ability_target_t *target) {
	zrc_host_t *zrc_host = zrc->user;

	ability_t *ability = &zrc->ability[ability_id];
	physics_t *physics = ZRC_GET(zrc, physics, caster_id);

	id_t proj_id = zrc_host_put(zrc_host, guid_create());
	float proj_speed = 250;

	cpVect front = cpvforangle(physics->angle);
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
		.ttl = 0.5f
	});
	ZRC_SPAWN(zrc, visual, proj_id, &(visual_t) {
		.color = color_random(255)
	});
	contact_damage_t contact_damage = {
		.damage = {
			.from = caster_id,
			.ability = ability_id,
			.health = 10*2
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
	physics_t *physics = ZRC_GET(zrc, physics, caster_id);

	id_t proj_id = zrc_host_put(zrc_host, guid_create());
	float proj_speed = 100;

	cpVect front = cpvforangle(physics->angle);
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
		.ttl = 1
	});
	ZRC_SPAWN(zrc, visual, proj_id, &(visual_t) {
		.color = color_random(255)
	});
	contact_damage_t contact_damage = {
		.damage = {
			.from = caster_id,
			.ability = ability_id,
			.health = 30*2
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
		.health = 20*2
	};
	ZRC_SEND(zrc, damage, target->unit, &damage);

	physics_t *target_physics = ZRC_GET(zrc, physics, target->unit);
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
	printf("zrc_host %zu\n", sizeof(zrc_host_t));
	srand((unsigned)(stm_now() & UINT32_MAX));

	zrc->user = zrc_host;

	zrc_host->entities = kh_init(ehash);
	kh_resize(ehash, zrc_host->entities, MAX_ENTITIES);

	zrc->ability[ABILITY_TUR_PROJ_ATTACK].cast = cast_tur_proj_attack;
	zrc->ability[ABILITY_BLINK].cast = cast_blink;
	zrc->ability[ABILITY_FIX_PROJ_ATTACK].cast = cast_fix_proj_attack;
	zrc->ability[ABILITY_TARGET_NUKE].cast = cast_target_nuke;

	demo_world_create(&zrc_host->demo_world, zrc_host, zrc);

	zrc_host->status = TF_NewStatus();
	zrc_host->graph = TF_NewGraph();

	//TF_SessionOptions* session_options = TF_NewSessionOptions();
	//zrc_host->session = TF_LoadSessionFromSavedModel(session_options, 0, "C:\\GitHub\\aumfer\\zrc-learn\\", (char*[]) { "serve" }, 1, zrc_host->graph, 0, zrc_host->status);
	//if (TF_GetCode(zrc_host->status) != TF_OK) {
	//	const char *msg = TF_Message(zrc_host->status);
	//	puts(msg);
	//}
	//TF_DeleteSessionOptions(session_options);

	TF_SessionOptions* session_options = TF_NewSessionOptions();
	zrc_host->session = TF_NewSession(zrc_host->graph, session_options, zrc_host->status);
	TF_DeleteSessionOptions(session_options);
	TF_ImportGraphDefOptions* import_options = TF_NewImportGraphDefOptions();
	FILE *f = fopen("C:\\GitHub\\aumfer\\zrc-learn\\test.pb", "rb");
	printf("%d", errno);
	fseek(f, 0, SEEK_END);
	int len = ftell(f);
	void *buf = malloc(len);
	fseek(f, 0, SEEK_SET);
	size_t read = fread(buf, 1, len, f);
	assert(read == len);
	TF_Buffer* graph_def = TF_NewBufferFromString(buf, len);
	free(buf);
	fclose(f);
	TF_GraphImportGraphDef(zrc_host->graph, graph_def, import_options, zrc_host->status);
	if (TF_GetCode(zrc_host->status) != TF_OK) {
		const char *msg = TF_Message(zrc_host->status);
		puts(msg);
	}
	TF_DeleteImportGraphDefOptions(import_options);
#if _DEBUG
	size_t pos = 0;
	TF_Operation *op;
	while (op = TF_GraphNextOperation(zrc_host->graph, &pos)) {
		const char *name = TF_OperationName(op);
		const char *type = TF_OperationOpType(op);
		int num_inputs = TF_OperationNumInputs(op);
		int num_outputs = TF_OperationNumOutputs(op);
		printf("%s %s %dx%d\n", name, type, num_inputs, num_outputs);
	}
#endif
	zrc_host->input.oper = TF_GraphOperationByName(zrc_host->graph, "input/Ob");
	zrc_host->input.index = 0;
	zrc_host->output.oper = TF_GraphOperationByName(zrc_host->graph, "output/add");
	//zrc_host->output.oper = TF_GraphOperationByName(zrc_host->graph, "model/split");
	//zrc_host->output.oper = TF_GraphOperationByName(zrc_host->graph, "model/Exp");
	//zrc_host->output.oper = TF_GraphOperationByName(zrc_host->graph, "output/strided_slice_1");
	zrc_host->output.index = 0;
	zrc_host->input_tensor = TF_AllocateTensor(TF_FLOAT, (int64_t[]){ 64, AI_OBSERVATION_LENGTH }, 2, 64 * AI_OBSERVATION_LENGTH * sizeof(float));
	zrc_host->output_tensor = TF_AllocateTensor(TF_FLOAT, (int64_t[]) { AI_ACTION_LENGTH }, 1, AI_ACTION_LENGTH * sizeof(float));

	//TF_Operation *init_op = TF_GraphOperationByName(zrc_host->graph, "init");
	//TF_SessionRun(zrc_host->session, NULL,
	//	/* No inputs */
	//	NULL, NULL, 0,
	//	/* No outputs */
	//	NULL, NULL, 0,
	//	/* Just the init operation */
	//	(TF_Operation*[]){init_op}, 1,
	//	/* No metadata */
	//	NULL, zrc_host->status);
	//if (TF_GetCode(zrc_host->status) != TF_OK) {
	//	const char *msg = TF_Message(zrc_host->status);
	//	puts(msg);
	//}

	timer_create(&zrc_host->timer);
}
void zrc_host_shutdown(zrc_host_t *zrc_host) {
	TF_DeleteTensor(zrc_host->output_tensor);
	TF_DeleteTensor(zrc_host->input_tensor);
	TF_DeleteSession(zrc_host->session, zrc_host->status);
	TF_DeleteGraph(zrc_host->graph);
	TF_DeleteStatus(zrc_host->status);
	kh_destroy(ehash, zrc_host->entities);
}

void zrc_host_tick(zrc_host_t *zrc_host, zrc_t *zrc) {
	timer_update(&zrc_host->timer);

	double dts = stm_sec(zrc_host->timer.dt);
	moving_average_update(&zrc_host->tick_fps, (float)dts);

	zrc_host->accumulator += dts;
	int frames = 0;
	while (zrc_host->accumulator >= TICK_RATE) {
		zrc_host->accumulator -= TICK_RATE;
		
		zrc_host_update(zrc_host, zrc);
	}
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
	
	if (1) {
	//if ((zrc_host->frame & 3) == 0) {
		for (int i = 0; i < MAX_ENTITIES; ++i) {
			ai_t *ai = ZRC_GET(zrc, ai, i);
			if (ai && !ai->train) {
				for (int j = 0; j < 64; ++j) {
					ai_observe(zrc, i, j, (float *)TF_TensorData(zrc_host->input_tensor)+(j*AI_OBSERVATION_LENGTH));
				}
				TF_SessionRun(zrc_host->session, 0,
					(TF_Output[]) { zrc_host->input },
					(TF_Tensor*[]) { zrc_host->input_tensor }, 1,
					(TF_Output[]) { zrc_host->output }, (TF_Tensor*[]) { zrc_host->output_tensor }, 1,
					0, 0, 0, zrc_host->status);
				if (TF_GetCode(zrc_host->status) != TF_OK) {
#if _DEBUG
					const char *msg = TF_Message(zrc_host->status);
					puts(msg);
#endif
					assert(0);
				}
				else {
					ai_act(zrc, i, TF_TensorData(zrc_host->output_tensor));
				}
			}
		}
	}
}

id_t zrc_host_put(zrc_host_t *zrc_host, guid_t guid) {
	int absent;
	khint_t k = kh_put(ehash, zrc_host->entities, guid, &absent);
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
	id_t radiant = zrc_host_put(zrc_host, guid_create());
	id_t dire = zrc_host_put(zrc_host, guid_create());
	//id_t other = zrc_host_put(&zrc_host, guid_create());

	demo_world->radiant = radiant;
	demo_world->dire = dire;

	printf("radiant %d\n", radiant);
	printf("dire %d\n", dire);

	ZRC_SPAWN(zrc, relate, radiant, &(relate_t){0});
	ZRC_SPAWN(zrc, relate, dire, &(relate_t){0});

	zrc->relate_change[zrc->num_relate_changes++] = (relationship_t) {
		.from = dire,
			.to = radiant,
			.amount = -10
	};

	const float SMALL_SHIP = 2.5f;
	const float MEDIUM_SHIP = 5;
	const float LARGE_SHIP = 12.5;
	const float CAPITAL_SHIP = 50;
	const int NUM_TEST_ENTITIES = 16;
	const int WORLD_FACTOR = 16;
	for (int i = 0; i < NUM_TEST_ENTITIES; ++i) {
		id_t id = zrc_host_put(zrc_host, guid_create());

		id_t faction;
		if (randf() > 0.5) {
			if (randf() > 0.5) {
				faction = radiant;
			}
			else {
				//faction = other;
				faction = radiant;
			}
		}
		else {
			if (randf() > 0.5) {
				faction = dire;
			}
			else {
				//faction = other;
				faction = dire;
			}
		}

		physics_t physics = {
			//.type = i && (randf() > 0.5) ? CP_BODY_TYPE_STATIC : CP_BODY_TYPE_DYNAMIC,
			.type = CP_BODY_TYPE_DYNAMIC,
			.collide_flags = ~0,
			.collide_mask = ~0,
			.response_mask = ~0,
			.max_speed = 150,
			.max_spin = 2,
			.damping = 0.05f,
			//.radius = 0.5f,
			//.radius = !i ? SMALL_SHIP : randf() * 12 + 0.5f,
			.radius = MEDIUM_SHIP,
			.position = {.x = randf() * WORLD_FACTOR * NUM_TEST_ENTITIES,.y = randf() * WORLD_FACTOR * NUM_TEST_ENTITIES },
			.angle = randf() * 2 * CP_PI
		};
		ZRC_SPAWN(zrc, physics, id, &physics);
		if (!i) {
			demo_world->player = id;
			//ZRC_SPAWN(zrc, physics_controller, id, &(physics_controller_t){0});
		} /*else*/ {
			ZRC_SPAWN(zrc, locomotion, id, &(locomotion_t){0});
			ZRC_SPAWN(zrc, seek, id, &(seek_t){0});
		}
		ZRC_SPAWN(zrc, visual, id, &(visual_t) {
			//.color = color_random(255)
			.color = faction == radiant ? 0xff0000ff : (faction == dire ? 0xff00ff00 : 0xffff0000)
		});
		flight_t flight = {
			.max_thrust = 15000,
			.max_turn = 15000
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
				[1].ability = ABILITY_BLINK,
				[2].ability = ABILITY_FIX_PROJ_ATTACK,
				[3].ability = ABILITY_TARGET_NUKE,
			}
		};
		ZRC_SPAWN(zrc, caster, id, &caster);
		ZRC_SPAWN(zrc, sense, id, &(sense_t) {
			.range = 250
		});
		if (!i) {
			printf("player %d team %d\n", id, faction);
		}
		ZRC_SPAWN(zrc, relate, id, &(relate_t){0});

		zrc->relate_change[zrc->num_relate_changes++] = (relationship_t) {
			.from = faction,
				.to = id,
				.amount = 1
		};

		ZRC_SPAWN(zrc, ai, id, &(ai_t){
			.train = !i
		});
	}
}
void demo_world_delete(demo_world_t *demo_world) {

}