#include <zrc_draw.h>

void zrc_draw_create(zrc_draw_t *zrc_draw) {
	control_create(&zrc_draw->control);
	draw_create(&zrc_draw->draw);
	timer_create(&zrc_draw->timer);
}
void zrc_draw_delete(zrc_draw_t *zrc_draw) {

}

void zrc_draw_frame(zrc_draw_t *zrc_draw, zrc_t *zrc) {
	timer_update(&zrc_draw->timer);
	float dt = (float)stm_sec(zrc_draw->timer.dt);

	ui_frame(&zrc_draw->ui);
	control_frame(&zrc_draw->control, &zrc_draw->ui, &zrc_draw->camera, zrc);
	camera_frame(&zrc_draw->camera, dt);

	draw_frame(&zrc_draw->draw, zrc, &zrc_draw->ui, &zrc_draw->control, &zrc_draw->camera, dt);
}
