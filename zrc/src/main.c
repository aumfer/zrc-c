#include <zrc.h>
#include <font.h>
#include <stdio.h>
#include <draw.h>
#include <camera.h>
#include <ui.h>
#include <control.h>
#include <tinycthread.h>
#include <zrc_host.h>

#define SOKOL_IMPL
#define SOKOL_WIN32_FORCE_MAIN
//#define SOKOL_D3D11
#define SOKOL_GLCORE33
#include <sokol_app.h>
#include <sokol_gfx.h>

static zrc_t noalloc;
static zrc_t *zrc = &noalloc;
static draw_t draw;
static camera_t camera;
static ui_t ui;
static control_t control;
static thrd_t thrd;
static timer_t timer;

static zrc_host_t zrc_host;

static int thread(void *_) {
	zrc_startup(zrc);
	zrc_host_startup(&zrc_host, zrc);

	const float SMALL_SHIP = 2.5f;
	const float LARGE_SHIP = 12.5;
	const float CAPITAL_SHIP = 50;
	for (int i = 0; i < 1024; ++i) {
		id_t id = zrc_host_put(&zrc_host, guid_create());

		physics_t physics = {
			.type = rand() > RAND_MAX / 2 ? CP_BODY_TYPE_STATIC : CP_BODY_TYPE_DYNAMIC,
			.collide_flags = ~0,
			.collide_mask = ~0,
			//.radius = 0.5f,
			.radius = !i ? SMALL_SHIP : randf() * 12 + 0.5f,
			.position = { .x = randf() * 4096, .y = randf() * 4096 },
			.angle = randf() * 2 * HMM_PI32
		};
		ZRC_SPAWN(zrc, physics, id, &physics);
		if (!i) {
			control.unit = id;
			//ZRC_SPAWN(zrc, physics_controller, id, &(physics_controller_t){0});
		}
		ZRC_SPAWN(zrc, visual, id, &(visual_t) {
			.color = color_random(255)
		});
		flight_t flight = {
			.max_thrust = 150,
			.max_turn = 5
		};
		ZRC_SPAWN(zrc, flight, id, &flight);
		life_t life = {
			.max_health = 100,
			.health = 75,
			.max_mana = 100,
			.mana = 25,
			.max_rage = 100,
			.rage = 50
		};
		ZRC_SPAWN(zrc, life, id, &life);
		caster_t caster = {
			.casts = {
				[0].ability = 1,
				[1].ability = 2
			}
		};
		ZRC_SPAWN(zrc, caster, id, &caster);
	}

	for (;;) {
		zrc_tick(zrc);
		zrc_host_tick(&zrc_host, zrc);
		thrd_yield();
	}
}

void init(void) {
	stm_setup();
	//_sapp_SwapIntervalEXT(0);

	timer_create(&timer);

	thrd_create(&thrd, thread, 0);

	draw_create(&draw);
}

void frame(void) {
	timer_update(&timer);
	float dt = (float)stm_sec(timer.dt);

	ui_frame(&ui);
	control_frame(&control, &ui, &camera, zrc);

	camera_frame(&camera, dt);

	draw_frame(&draw, zrc, &ui, &control, &camera, dt);
}

void cleanup(void) {
	zrc_shutdown(zrc);
	sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
	return (sapp_desc) {
		.init_cb = init,
		.frame_cb = frame,
		.cleanup_cb = cleanup,
		.user_data = &ui,
		.event_userdata_cb = ui_event_cb,
		.width = 1920/2,
		.height = 1080/2,
		.sample_count = 4,
		.window_title = "-= zen rat city =-",
	};
}
