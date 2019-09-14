#include <control.h>
#include <zmath.h>

void control_update(control_t *control, const ui_t *ui, camera_t *camera, zrc_t *zrc) {
	camera->zoom = 64 - ui_touch(ui, UI_TOUCH_SCROLL).point[1];

	float viewport[4] = { 0, 0, (float)sapp_width(), (float)sapp_height() };
	ui_touchstate_t pointer = ui_touch(ui, UI_TOUCH_POINTER);
	hmm_vec3 pick_start = hmm_unproject(HMM_Vec3(pointer.point[0], pointer.point[1], -1), camera->view_projection, viewport);
	hmm_vec3 pick_end = hmm_unproject(HMM_Vec3(pointer.point[0], pointer.point[1], 1), camera->view_projection, viewport);

	//float mx = snorm(pointer.point[0] / sapp_width());
	//float my = snorm(pointer.point[1] / sapp_width());
	//hmm_vec4 screen = HMM_Vec4(mx, -my, 1, 1);
	//hmm_vec4 world = HMM_MultiplyMat4ByVec4(camera->inv_view_projection, screen);

	control->hover = physics_query_ray(zrc, (float[2]) { pick_start.X, pick_start.Y }, (float[2]) { pick_end.X, pick_end.Y }, 1);

	hmm_vec3 ro = pick_end;
	hmm_vec3 rd = HMM_NormalizeVec3(HMM_SubtractVec3(pick_start, pick_end));
	float worldt = isect_plane(ro, rd, HMM_Vec4(0, 0, 1, 0));
	hmm_vec3 worldp = HMM_AddVec3(ro, HMM_MultiplyVec3f(rd, worldt));

	control->ground[0] = worldp.X;
	control->ground[1] = worldp.Y;

	physics_t *physics = ZRC_GET(zrc, physics, control->select);
	if (physics) {
		camera->zoom += physics->radius;

		cpVect look = cpv(0, 16);
		cpVect target = cpvrotate(look, cpvforangle(physics->angle));
		camera->target[0] = (float)(physics->position[0] + target.x);
		camera->target[1] = (float)(physics->position[1] + target.y);
		cpVect offset = cpv(0, -16);
		cpVect position = cpvrotate(offset, cpvforangle(physics->angle));
		camera->position[0] = (float)(physics->position[0] + position.x);
		camera->position[1] = (float)(physics->position[1] + position.y);
	}

	flight_t *flight = ZRC_GET_WRITE(zrc, flight, control->select);
	if (flight) {
		flight->thrust[0] = 0;
		flight->thrust[1] = 0;
		if (ui_button(ui, SAPP_KEYCODE_W)) {
			flight->thrust[1] += 1;
		}
		if (ui_button(ui, SAPP_KEYCODE_S)) {
			flight->thrust[1] -= 1;
		}
		flight->turn = 0;
		if (ui_button(ui, SAPP_KEYCODE_A)) {
			flight->turn += 1;
		}
		if (ui_button(ui, SAPP_KEYCODE_D)) {
			flight->turn -= 1;
		}
	}

	caster_t *caster = ZRC_GET_WRITE(zrc, caster, control->select);
	if (caster) {
		for (int i = 0; i < CASTER_MAX_ABILITIES; ++i) {
			const ability_t *ability = &zrc->ability[caster->abilities[i].ability];
			if ((ability->target_flags & ABILITY_TARGET_POINT) == ABILITY_TARGET_POINT) {
				caster->abilities[i].target.point[0] = control->ground[0];
				caster->abilities[i].target.point[1] = control->ground[1];
			}
			if ((ability->target_flags & ABILITY_TARGET_UNIT) == ABILITY_TARGET_UNIT) {
				caster->abilities[i].target.unit = control->hover;
			}
		}
	}
}