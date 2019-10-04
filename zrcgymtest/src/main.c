#include <env.h>
#include <tinycthread.h>

int main(int argc, char **argv) {
	env_t *env = env_create();

	rl_obs_t obs = { 0 };
	env_reset(env, &obs);
	env_render(env);

	thrd_sleep(&(struct timespec) {
		.tv_sec = 1
	}, 0);

	env_reset(env, &obs);

	gym_t *gym = env_gym(env);
	zrc_t *zrc = &gym->zrc;
	id_t agent = gym->agent;

	for (;;) {
		rl_observe(zrc, agent, &obs);

		thrd_yield();
	}
    return 0;
}