#include <control.h>
#include <zmath.h>

void control_create(control_t *control) {
	control->target = ID_INVALID;
	control->unit = ID_INVALID;
}
void control_delete(control_t *control) {

}

void control_frame(control_t *control, const ui_t *ui, camera_t *camera, zrc_t *zrc) {
	camera->zoom = 64 - ui_touch(ui, UI_TOUCH_SCROLL).y;

	float viewport[4] = { 0, 0, (float)sapp_width(), (float)sapp_height() };
	ui_touchpoint_t pointer = ui_touch(ui, UI_TOUCH_POINTER);
	hmm_vec3 pick_start = hmm_unproject(HMM_Vec3(pointer.x, pointer.y, 0), camera->view_projection, viewport);
	hmm_vec3 pick_end = hmm_unproject(HMM_Vec3(pointer.x, pointer.y, 1), camera->view_projection, viewport);

	//float mx = snorm(pointer.point[0] / sapp_width());
	//float my = snorm(pointer.point[1] / sapp_width());
	//hmm_vec4 screen = HMM_Vec4(mx, -my, 1, 1);
	//hmm_vec4 world = HMM_MultiplyMat4ByVec4(camera->inv_view_projection, screen);

	hmm_vec3 ro = pick_start;
	//ro = HMM_Vec3(camera->position[0], camera->position[1], camera->zoom);
	hmm_vec3 rd = HMM_NormalizeVec3(HMM_SubtractVec3(pick_end, pick_start));
	float worldt = isect_plane(ro, rd, HMM_Vec4(0, 0, 1, 0));
	hmm_vec3 worldp = HMM_AddVec3(ro, HMM_MultiplyVec3f(rd, worldt));

	//pick_start = HMM_Vec3(camera->position[0], camera->position[1], camera->zoom);
	//id_t target = physics_query_ray(zrc, cpv(pick_start.X, pick_start.Y), cpv(pick_end.X, pick_end.Y), 1);
	id_t target = physics_query_point(zrc, cpv(worldp.X, worldp.Y), 2);
	if (target != ID_INVALID && target != control->unit) {
		control->target = target;
	}

	control->ground[0] = worldp.X;
	control->ground[1] = worldp.Y;

	const physics_t *physics = ZRC_GET(zrc, physics, control->unit);
	if (physics) {
		camera->zoom += physics->radius;

		cpVect look = cpv(16, 0);
		if (!control->fixed_camera) {
			look = cpvrotate(look, physics->front);
		}
		camera->target[0] = (float)(physics->position.x /*+ physics->velocity[0]/10*/ + look.x);
		camera->target[1] = (float)(physics->position.y /*+ physics->velocity[1]/10*/ + look.y);
		cpVect offset = cpv(-16, 0);
		if (!control->fixed_camera) {
			offset = cpvrotate(offset, physics->front);
		}
		camera->position[0] = (float)(physics->position.x /*- physics->velocity[0]/10*/ + offset.x);
		camera->position[1] = (float)(physics->position.y /*- physics->velocity[1]/10*/ + offset.y);
	}

	if (ZRC_HAS(zrc, flight, control->unit)) {
		flight_thrust_t flight_thrust = { 0 };

		if (ui_button(ui, SAPP_KEYCODE_SPACE)) {
			flight_thrust.damp = 0;
		} else {
			flight_thrust.damp = SHIP_DAMPING;
		}

		if (ui_button(ui, CONTROL_BUTTON_FORWARD)) {
			flight_thrust.thrust.x += 1;
		}
		if (ui_button(ui, CONTROL_BUTTON_BACKWARD)) {
			flight_thrust.thrust.x -= 1;
		}
		if (ui_button(ui, CONTROL_BUTTON_STRAFE_LEFT)) {
			flight_thrust.thrust.y += 1;
		}
		if (ui_button(ui, CONTROL_BUTTON_STRAFE_RIGHT)) {
			flight_thrust.thrust.y -= 1;
		}
		flight_thrust.turn = 0;
		if (ui_button(ui, CONTROL_BUTTON_LEFT)) {
			flight_thrust.turn += 1;
		}
		if (ui_button(ui, CONTROL_BUTTON_RIGHT)) {
			flight_thrust.turn -= 1;
		}
		ZRC_SEND(zrc, flight_thrust, control->unit, &flight_thrust);
	}

	if (ZRC_HAS(zrc, caster, control->unit)) {
		const caster_t *caster = ZRC_GET(zrc, caster, control->unit);
		int cast_buttons[] = {
			CONTROL_BUTTON_CAST0,
			CONTROL_BUTTON_CAST1,
			CONTROL_BUTTON_CAST2,
			CONTROL_BUTTON_CAST3,
			CONTROL_BUTTON_CAST4,
			CONTROL_BUTTON_CAST5,
			CONTROL_BUTTON_CAST6,
			CONTROL_BUTTON_CAST7,
		};
		for (int i = 0; i < CASTER_MAX_ABLITIES; ++i) {
			const caster_ability_t *caster_ability = &caster->abilities[i];
			const ability_t *ability = &zrc->ability[caster_ability->ability];
			cast_t cast = {
				.caster_ability = i
			};
			if ((ability->target_flags & ABILITY_TARGET_POINT) == ABILITY_TARGET_POINT) {
				cast.target.point[0] = control->ground[0];
				cast.target.point[1] = control->ground[1];
			}
			if ((ability->target_flags & ABILITY_TARGET_UNIT) == ABILITY_TARGET_UNIT) {
				cast.target.unit = control->target;
			}

			if (i < _countof(cast_buttons)) {
				if (ui_button(ui, cast_buttons[i])) {
					cast.cast_flags |= CAST_WANTCAST;
				} else {
					cast.cast_flags &= ~CAST_WANTCAST;
				}
			}

			ZRC_SEND(zrc, cast, control->unit, &cast);
		}
	}

	if (ui_button(ui, SAPP_MOUSEBUTTON_MIDDLE)) {
		seek_to_t seek_to = {
			.point = cpv(control->ground[0], control->ground[1])
		};
		ZRC_SEND(zrc, seek_to, control->unit, &seek_to);
	}
}