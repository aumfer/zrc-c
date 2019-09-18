#include <env.h>
#include <tinycthread.h>

int main(int argc, char **argv) {
	env_t *env = env_create();
	float obs[AI_OBSERVATION_LENGTH];
	env_reset(env, obs);
	env_render(env);

	thrd_sleep(&(struct timespec) {
		.tv_sec = 5
	}, 0);

	env_reset(env, obs);

	for (;;) {
		thrd_yield();
	}
    return 0;
}