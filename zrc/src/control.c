#include <control.h>

void control_update(control_t *control, const ui_t *ui, camera_t *camera, zrc_t *zrc) {
	camera->zoom = 40 - ui_touch(ui, UI_TOUCH_SCROLL).point[1];

	physics_t *physics = ZRC_GET(zrc, physics, control->select);
	if (physics) {
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
}