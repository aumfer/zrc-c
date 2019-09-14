#include <draw.h>
#include <stdio.h>

void draw_create(draw_t *draw) {
	sg_setup(&(sg_desc) {
		.mtl_device = sapp_metal_get_device(),
			.mtl_renderpass_descriptor_cb = sapp_metal_get_renderpass_descriptor,
			.mtl_drawable_cb = sapp_metal_get_drawable,
			.d3d11_device = sapp_d3d11_get_device(),
			.d3d11_device_context = sapp_d3d11_get_device_context(),
			.d3d11_render_target_view_cb = sapp_d3d11_get_render_target_view,
			.d3d11_depth_stencil_view_cb = sapp_d3d11_get_depth_stencil_view
	});

	font_create(&draw->font, FONT_CONSOLAS_16);
	draw_visual_create(&draw->draw_visual);
	draw_world_create(&draw->draw_world);
}
void draw_delete(draw_t *draw) {

}

void draw_update(draw_t *draw, zrc_t *zrc, const ui_t *ui, const control_t *control, const camera_t *camera) {
	float dt = (float)stm_sec(stm_since(zrc->time));

	font_begin(&draw->font);

	char fps[32];
	sprintf(fps, "%.0fms %.0ffps (%.2fus)", 1000 * zrc->fps.avg, 1.0f / zrc->fps.avg, dt*1000);
	font_print(&draw->font, fps, (float[2]) { [0] = 10, [1] = 10 }, 0xff333333);
	font_print(&draw->font, fps, (float[2]) { [0] = 11, [1] = 11 }, 0xffcccccc);

	ui_touchstate_t pointer = ui_touch(ui, UI_TOUCH_POINTER);
	char ptr[32];
	sprintf(ptr, "pointer: %.0f %.0f (%d)", pointer.point[0], pointer.point[1], control->hover);
	font_print(&draw->font, ptr, (float[2]) { [0] = 10, [1] = 30 }, 0xff333333);
	font_print(&draw->font, ptr, (float[2]) { [0] = 11, [1] = 31 }, 0xffcccccc);

	char grd[32];
	sprintf(grd, "ground: %.0f %.0f", control->ground[0], control->ground[1]);
	font_print(&draw->font, grd, (float[2]) { [0] = 10, [1] = 50 }, 0xff333333);
	font_print(&draw->font, grd, (float[2]) { [0] = 11, [1] = 51 }, 0xffcccccc);

	physics_t *physics = ZRC_GET(zrc, physics, control->select);
	if (physics) {
		char pos[32];
		sprintf(pos, "ship: %.0f %.0f", physics->position[0], physics->position[1]);
		font_print(&draw->font, pos, (float[2]) { [0] = 10, [1] = 70 }, 0xff333333);
		font_print(&draw->font, pos, (float[2]) { [0] = 11, [1] = 71 }, 0xffcccccc);

		char spd[32];
		cpFloat speed = cpvlength(cpv(physics->velocity[0], physics->velocity[1]));
		sprintf(spd, "speed: %.0f %.0f", speed, fabs(physics->angular_velocity));
		font_print(&draw->font, spd, (float[2]) { [0] = 10, [1] = 90 }, 0xff333333);
		font_print(&draw->font, spd, (float[2]) { [0] = 11, [1] = 91 }, 0xffcccccc);
	}

	if (control->hover != ID_INVALID) {
		physics_t *physics = ZRC_GET(zrc, physics, control->hover);

		char hov[32];
		sprintf(hov, "hov: %.0f %.0f", physics->position[0], physics->position[1]);
		font_print(&draw->font, hov, (float[2]) { [0] = 10, [1] = 110 }, 0xff333333);
		font_print(&draw->font, hov, (float[2]) { [0] = 11, [1] = 111 }, 0xffcccccc);
	}

	draw_world_tick(&draw->draw_world, camera);
	draw_visual_tick(&draw->draw_visual, zrc, camera, control, dt);

	font_end(&draw->font);
	font_draw(&draw->font);
}