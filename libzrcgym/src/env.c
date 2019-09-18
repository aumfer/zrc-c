#include <env.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct env {
	unsigned num_resets;
	gym_t *gym;
} env_t;

static int has_stm;

env_t *env_create(void) {
	env_t *env = calloc(1, sizeof(env_t));
	return env;
}
void env_delete(env_t *env) {
	gym_delete(env->gym);
	free(env);
}

int env_observation_length(void) {
	return AI_OBSERVATION_LENGTH;
}
int env_action_length(void) {
	return AI_ACTION_LENGTH;
}

void env_reset(env_t *env, float *observation) {
	if (!has_stm) {
		stm_setup();
		has_stm = 1;
	}
	++env->num_resets;
	printf("env %u\n", env->num_resets);
	if (env->gym) {
		gym_delete(env->gym);
		free(env->gym);
		env->gym = 0;
	}
	env->gym = calloc(1, sizeof(gym_t));
	gym_create(env->gym);

	for (int i = 0; i < 3; ++i) {
		gym_update(env->gym);
	}

	ai_observe(&env->gym->zrc, env->gym->agent, 0, observation);
}

void env_step(env_t *env, float *action, float *observation, float *reward, int *done) {
	if (!env->gym) return;
	gym_t *gym = env->gym;
	zrc_t *zrc = &env->gym->zrc;
	id_t agent = env->gym->agent;

	for (int i = 0; i < 3; ++i) {
		//printf("%u act", zrc.frame);
		ai_act(zrc, agent, action);
		//puts(" done");
		//printf("%u update", zrc.frame);
		gym_update(gym);
		//puts(" done");

		ai_t *ai = ZRC_GET(zrc, ai, agent);
		if (ai) {
			*reward += ai->reward;
		} else {
			*done = 1;
			ai_t *aip = ZRC_GET_PREV(zrc, ai, agent);
			printf("done %.2f\n", aip->total_reward);
			return;
		}
	}

	//printf("%u obs", zrc.frame);
	ai_observe(zrc, agent, 0, observation);
	//puts(" done");
}