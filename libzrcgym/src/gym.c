#include <gym.h>

void gym_create(gym_t *gym) {
	zrc_startup(&gym->zrc);
	zrc_host_startup(&gym->zrc_host, &gym->zrc);
	ai_startup(&gym->ai);

	gym->agent = gym->zrc_host.demo_world.player;
}
void gym_delete(gym_t *gym) {
	ai_shutdown(&gym->ai);
	zrc_host_shutdown(&gym->zrc_host);
	zrc_shutdown(&gym->zrc);
}
void gym_update(gym_t *gym) {
	zrc_update(&gym->zrc);
	zrc_host_update(&gym->zrc_host, &gym->zrc);
	ai_update(&gym->zrc, &gym->ai);
}