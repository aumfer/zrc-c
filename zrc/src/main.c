#include <zrc.h>
#include <font.h>
#include <stdio.h>
#include <tinycthread.h>
#include <zrc_host.h>
#include <zrc_draw.h>

#define SOKOL_IMPL
#define SOKOL_WIN32_FORCE_MAIN
//#define SOKOL_D3D11
#define SOKOL_GLCORE33
#include <sokol_app.h>
#include <sokol_gfx.h>

static zrc_t zrc;
static zrc_host_t zrc_host;
static zrc_draw_t zrc_draw;
static thrd_t thrd;

static int quit;

static int thread(void *_) {
	zrc_startup(&zrc);
	zrc_host_startup(&zrc_host, &zrc);

	zrc_draw.control.unit = zrc_host.demo_world.player;

	// todo update_timer, accumulator, call zrc_update, remove zrc_tick

	while (!quit) {
		zrc_tick(&zrc);
		zrc_host_tick(&zrc_host, &zrc);

		thrd_yield();
	}

	zrc_shutdown(&zrc);

	return 0;
}

static void init(void) {
	stm_setup();
	//_sapp_SwapIntervalEXT(0);

	thrd_create(&thrd, thread, 0);

	zrc_draw_create(&zrc_draw);
}

static void frame(void) {
	zrc_draw_frame(&zrc_draw, &zrc);
}

static void cleanup(void) {
	quit = 1;
	zrc_draw_delete(&zrc_draw);
	int res;
	thrd_join(&thrd, &res);
}

sapp_desc sokol_main(int argc, char* argv[]) {
	return (sapp_desc) {
		.init_cb = init,
		.frame_cb = frame,
		.cleanup_cb = cleanup,
		.user_data = &zrc_draw.ui,
		.event_userdata_cb = ui_event_cb,
		.width = 1920/2,
		.height = 1080/2,
		.sample_count = 4,
		.window_title = "-= zen rat city =-",
	};
}
