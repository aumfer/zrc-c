#include <zrc.h>
#include <font.h>
#include <stdio.h>
#include <draw_visual.h>

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
}

void draw_frame() {
	draw_visual_tick(&draw_visual, zrc);
	font_draw(&font);
}

void init(void) {
	_sapp_SwapIntervalEXT(0);

	zrc_startup(zrc);

	for (int i = 0; i < 1024; ++i) {
		physics_t physics = {
			.position = { [0] = ((float)rand() / RAND_MAX) * sapp_width(), [1] = ((float)rand() / RAND_MAX) * sapp_height() },
			.radius = 0.5f,
			//.velocity = { [0] = 10, [1] = 10 }
		};
		ZRC_SPAWN(zrc, physics, i, &physics);
		ZRC_SPAWN(zrc, visual, i, &(visual_t) {
			//.color = 0xff0000ff
			.color = rgba(0, 255, 0, 255)
		});
	}

	draw_init();
}

void frame(void) {
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		physics_t *physics = ZRC_GET(zrc, physics, i);
		if (physics) {
			physics->force[0] = ((float)rand() / RAND_MAX - 0.5f) * 10;
			physics->force[1] = ((float)rand() / RAND_MAX - 0.5f) * 10;
			physics->torque = 1;
		}
	}

	zrc_tick(zrc);

	font_begin(&font);
	char fps[32];
	sprintf(fps, "%.0fms %.2ffps", 1000 * zrc->fps.avg, 1.0f / zrc->fps.avg);
	font_print(&font, fps, (float[2]) { [0] = 10, [1] = 10 }, 0xff333333);
	font_print(&font, fps, (float[2]) { [0] = 11, [1] = 11 }, 0xffcccccc);
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
		.width = 1920/2,
		.height = 1080/2,
		.window_title = "-= zen rat city =-",
	};
}
