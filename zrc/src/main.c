#include <zrc.h>
#include <font.h>
#include <stdio.h>
#include <draw_visual.h>
#include <camera.h>
#include <ui.h>
#include <control.h>
#include <draw_world.h>

#define SOKOL_IMPL
#define SOKOL_WIN32_FORCE_MAIN
//#define SOKOL_D3D11
#define SOKOL_GLCORE33
#include <sokol_app.h>
#include <sokol_gfx.h>

static zrc_t noalloc;
static zrc_t *zrc = &noalloc;
static font_t font;
static draw_visual_t draw_visual;
static camera_t camera;
static ui_t ui;
static control_t control;
static draw_world_t draw_world;

void draw_init() {
	sg_setup(&(sg_desc) {
		.mtl_device = sapp_metal_get_device(),
		.mtl_renderpass_descriptor_cb = sapp_metal_get_renderpass_descriptor,
		.mtl_drawable_cb = sapp_metal_get_drawable,
		.d3d11_device = sapp_d3d11_get_device(),
		.d3d11_device_context = sapp_d3d11_get_device_context(),
		.d3d11_render_target_view_cb = sapp_d3d11_get_render_target_view,
		.d3d11_depth_stencil_view_cb = sapp_d3d11_get_depth_stencil_view
	});

	font_create(&font, FONT_CONSOLAS_16);
	draw_visual_create(&draw_visual);
	draw_world_create(&draw_world);
}

void draw_frame() {
	draw_world_tick(&draw_world, &camera);
	draw_visual_tick(&draw_visual, zrc, &camera, &control);
	font_draw(&font);
}

void init(void) {
	_sapp_SwapIntervalEXT(0);

	zrc_startup(zrc);

	const float SMALL_SHIP = 2.5f;
	const float LARGE_SHIP = 12.5;
	const float CAPITAL_SHIP = 50;
	for (int i = 0; i < 1024; ++i) {
		physics_t physics = {
			.type = rand() > RAND_MAX/2 ? CP_BODY_TYPE_STATIC : CP_BODY_TYPE_DYNAMIC,
			.collide_flags = ~0,
			.collide_mask = ~0,
			//.radius = 0.5f,
			.radius = !i ? SMALL_SHIP : randf() * 12 + 0.5f,
			.position = { [0] = randf() * 4096, [1] = randf() * 4096 },
			.angle = randf() * 2 * HMM_PI32
		};
		ZRC_SPAWN(zrc, physics, i, &physics);
		if (!i) {
			//ZRC_SPAWN(zrc, physics_controller, i, &(physics_controller_t){0});
		}
		ZRC_SPAWN(zrc, visual, i, &(visual_t) {
			.color = color_random(255)
		});
		flight_t flight = {
			.max_thrust = 100,
			.max_turn = 5
		};
		ZRC_SPAWN(zrc, flight, i, &flight);
		life_t life = {
			.max_health = 100,
			.health = 75,
			.max_mana = 100,
			.mana = 25,
			.max_rage = 100,
			.rage = 50
		};
		ZRC_SPAWN(zrc, life, i, &life);
	}

	draw_init();
}

void frame(void) {
	zrc_tick(zrc);

	ui_update(&ui);
	control_update(&control, &ui, &camera, zrc);

	camera_update(&camera);

	font_begin(&font);

	char fps[32];
	sprintf(fps, "%.0fms %.2ffps", 1000 * zrc->fps.avg, 1.0f / zrc->fps.avg);
	font_print(&font, fps, (float[2]) { [0] = 10, [1] = 10 }, 0xff333333);
	font_print(&font, fps, (float[2]) { [0] = 11, [1] = 11 }, 0xffcccccc);

	physics_t *physics = ZRC_GET(zrc, physics, control.select);
	if (physics) {
		char pos[32];
		sprintf(pos, "x: %.0f y: %.0f", physics->position[0], physics->position[1]);
		font_print(&font, pos, (float[2]) { [0] = 10, [1] = 30 }, 0xff333333);
		font_print(&font, pos, (float[2]) { [0] = 11, [1] = 31 }, 0xffcccccc);

		char spd[32];
		cpFloat speed = cpvlength(cpv(physics->velocity[0], physics->velocity[1]));
		sprintf(spd, "speed: %.0f %.0f", speed, fabs(physics->angular_velocity));
		font_print(&font, spd, (float[2]) { [0] = 10, [1] = 50 }, 0xff333333);
		font_print(&font, spd, (float[2]) { [0] = 11, [1] = 51 }, 0xffcccccc);
	}

	font_end(&font);

	draw_frame(zrc);
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
