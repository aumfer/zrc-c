#include <draw.h>
#include <stdio.h>
#include <inttypes.h>

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
	draw_locomotion_create(&draw->draw_locomotion);
}
void draw_delete(draw_t *draw) {
	sg_shutdown();
}

void draw_frame(draw_t *draw, zrc_t *zrc, const ui_t *ui, const control_t *control, const camera_t *camera, float dt) {
	moving_average_update(&draw->fps, dt);

	font_begin(&draw->font);

	char fps[32];
	sprintf_s(fps, sizeof(fps), "%.0ffps %0.0fms %.0f", 1.0 / draw->fps.avg, zrc->update_fps.avg * 1000, zrc->frame * TICK_RATE);
	font_print(&draw->font, fps, (float[2]) { [0] = 10, [1] = 10 }, 0xff333333);
	font_print(&draw->font, fps, (float[2]) { [0] = 11, [1] = 11 }, 0xffcccccc);

	ui_touchpoint_t pointer = ui_touch(ui, UI_TOUCH_POINTER);
	char ptr[32];
	sprintf_s(ptr, sizeof(ptr), "point: %.0f %.0f (%d)", pointer.x, pointer.y, control->target);
	font_print(&draw->font, ptr, (float[2]) { [0] = 10, [1] = 30 }, 0xff333333);
	font_print(&draw->font, ptr, (float[2]) { [0] = 11, [1] = 31 }, 0xffcccccc);

	char grd[64];
	sprintf_s(grd, sizeof(grd), "ground: %.0f %.0f", control->ground[0], control->ground[1]);
	font_print(&draw->font, grd, (float[2]) { [0] = 10, [1] = 50 }, 0xff333333);
	font_print(&draw->font, grd, (float[2]) { [0] = 11, [1] = 51 }, 0xffcccccc);

	physics_t *physics = ZRC_GET(zrc, physics, control->unit);
	if (physics) {
		char pos[32];
		sprintf_s(pos, sizeof(pos), "ship: %.0f %.0f", physics->position.x, physics->position.y);
		font_print(&draw->font, pos, (float[2]) { [0] = 10, [1] = 70 }, 0xff333333);
		font_print(&draw->font, pos, (float[2]) { [0] = 11, [1] = 71 }, 0xffcccccc);

		char spd[32];
		cpFloat speed = cpvlength(physics->velocity);
		sprintf_s(spd, sizeof(spd), "speed: %.2f %.2f", speed, fabs(physics->angular_velocity));
		font_print(&draw->font, spd, (float[2]) { [0] = 10, [1] = 90 }, 0xff333333);
		font_print(&draw->font, spd, (float[2]) { [0] = 11, [1] = 91 }, 0xffcccccc);
	}

	if (control->target != ID_INVALID) {
		physics_t *physics = ZRC_GET(zrc, physics, control->target);
		if (physics) {
			char hov[32];
			sprintf_s(hov, sizeof(hov), "hov: %.0f %.0f", physics->position.x, physics->position.y);
			font_print(&draw->font, hov, (float[2]) { [0] = 10, [1] = 110 }, 0xff333333);
			font_print(&draw->font, hov, (float[2]) { [0] = 11, [1] = 111 }, 0xffcccccc);
		}

		if (control->target != control->unit) {
			//float my_relation = relate_to_query(zrc, control->unit, control->target);
			//float their_relation = relate_to_query(zrc, control->target, control->unit);
			team_t *my_team = ZRC_GET(zrc, team, control->unit);
			team_t *their_team = ZRC_GET(zrc, team, control->target);
			if (my_team && their_team) {
				char rel[32];
				sprintf_s(rel, sizeof(rel), "rel: %d %d", *my_team, *their_team);
				font_print(&draw->font, rel, (float[2]) { [0] = 10, [1] = 130 }, 0xff333333);
				font_print(&draw->font, rel, (float[2]) { [0] = 11, [1] = 131 }, 0xffcccccc);
			}
		}
	}

	sense_t *sense = ZRC_GET(zrc, sense, control->unit);
	if (sense) {
		char numsense[32];
		sprintf_s(numsense, sizeof(numsense), "sense: %d", sense->num_entities);
		font_print(&draw->font, numsense, (float[2]) { [0] = 10, [1] = 150 }, 0xff333333);
		font_print(&draw->font, numsense, (float[2]) { [0] = 11, [1] = 151 }, 0xffcccccc);
	}

	ai_t *ai = ZRC_GET(zrc, ai, control->unit);
	if (ai) {
		char sreward[32];
		//float reward = ai->train_locomotion.reward;
		//float total_reward = ai->train_locomotion.total_reward;
		float reward = ai->train_sense.reward;
		float total_reward = ai->train_sense.total_reward;
		sprintf_s(sreward, sizeof(sreward), "reward: %4.2f %4.2f", reward, total_reward);
		font_print(&draw->font, sreward, (float[2]) { [0] = 10, [1] = 170 }, 0xff333333);
		font_print(&draw->font, sreward, (float[2]) { [0] = 11, [1] = 171 }, 0xffcccccc);
	}

	for (int i = 0; i < zrc_component_count; ++i) {
		char ctime[32];
		sprintf_s(ctime, sizeof(ctime), "%2d| %2.2fms", i, stm_ms(zrc->times[i]));
		font_print(&draw->font, ctime, (float[2]) { [0] = sapp_width() - 101.0f, [1] = sapp_height() - 20.0f - (20*i) }, 0xff333333);
		font_print(&draw->font, ctime, (float[2]) { [0] = sapp_width() - 100.0f, [1] = sapp_height() - 21.0f - (20*i) }, 0xffcccccc);
	}

	float extra = draw->not_real_time ? 0 : (float)stm_sec(stm_since(zrc->timer.time));
	draw_world_frame(&draw->draw_world, camera);
	draw_locomotion_frame(&draw->draw_locomotion, zrc, camera, control, dt);
	draw_visual_frame(&draw->draw_visual, zrc, camera, control, dt, extra);

	font_end(&draw->font);
	font_draw(&draw->font);

	if (dt > TICK_RATE*1.5f) {
		//puts("drop frame");
	}
}