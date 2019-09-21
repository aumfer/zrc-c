#include <gym.h>
#include <stdio.h>

void gym_create(gym_t *gym) {
	//puts("zrc_startup");
	zrc_startup(&gym->zrc);
	//puts("zrc_host_startup");
	zrc_host_startup(&gym->zrc_host, &gym->zrc);

	gym->agent = gym->zrc_host.demo_world.test_entities[rand() & (NUM_TEST_ENTITIES-1-1/*player*/)];
}
void gym_delete(gym_t *gym) {
	//puts("zrc_host_shutdown");
	zrc_host_shutdown(&gym->zrc_host);
	//puts("zrc_shutdown");
	zrc_shutdown(&gym->zrc);
}
void gym_update(gym_t *gym) {
	//printf("%u zrc_update", gym->zrc.frame);
	zrc_update(&gym->zrc);
	//puts(" done");
	//printf("%u zrc_host_update", gym->zrc.frame);
	zrc_host_update(&gym->zrc_host, &gym->zrc);	
	//puts(" done");
}