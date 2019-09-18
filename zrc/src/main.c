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
static timer_t frame_timer;

static zrc_host_t zrc_host;

static int thread(void *_) {
	zrc_startup(zrc);
	zrc_host_startup(&zrc_host, zrc);

	control.unit = zrc_host.demo_world.player;

	// todo update_timer, accumulator, call zrc_update, remove zrc_tick

	for (;;) {
		zrc_tick(zrc);
		zrc_host_tick(&zrc_host, zrc);

		thrd_yield();
	}

	zrc_shutdown(zrc);
}

void init(void) {
	stm_setup();
	//_sapp_SwapIntervalEXT(0);

	control_create(&control);

	thrd_create(&thrd, thread, 0);

	draw_create(&draw);
}

void frame(void) {
	timer_update(&frame_timer);
	float dt = (float)stm_sec(frame_timer.dt);

	ui_frame(&ui);
	control_frame(&control, &ui, &camera, zrc);

	camera_frame(&camera, dt);

	draw_frame(&draw, zrc, &ui, &control, &camera, dt);
}

void cleanup(void) {
	control_delete(&control);
	draw_delete(&draw);
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
