#include <env.h>
#include <tinycthread.h>

int main(int argc, char **argv) {
	env_t *env = env_create();

	env_reset(env);
	env_render(env);

	thrd_sleep(&(struct timespec) {
		.tv_sec = 1
	}, 0);

	env_reset(env);

	gym_t *gym = env_gym(env);
	zrc_t *zrc = &gym->zrc;

	for (;;) {
		float lobs[AI_LOCOMOTION_OBS_LENGTH];
		ai_observe_locomotion_train(zrc, gym->agent, lobs);

		float sobs[AI_SENSE_OBS_LENGTH];
		ai_observe_sense(zrc, gym->agent, sobs);

		float reward = 0;
		int done = 0;
		env_step_sense(env, lobs, sobs, &reward, &done);

		thrd_yield();
	}
    return 0;
}