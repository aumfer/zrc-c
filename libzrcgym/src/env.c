#include <env.h>
#include <stdio.h>

static unsigned num_resets = 0;
static gym_t *gym;

int env_observation_length(void) {
	return AI_OBSERVATION_LENGTH;
}
int env_action_length(void) {
	return AI_ACTION_LENGTH;
}

void env_reset(float *observation) {
	if (!num_resets) {
		stm_setup();
	}
	++num_resets;
	printf("env %u\n", num_resets);
	if (gym) {
		gym_delete(gym);
		free(gym);
		gym = 0;
	}
	gym = calloc(1, sizeof(gym_t));
	gym_create(gym);

	for (int i = 0; i < 3; ++i) {
		gym_update(gym);
	}

	ai_observe(&gym->zrc, gym->agent, observation);
}

void env_step(float *action, float *observation, float *reward, int *done) {
	if (!gym) return;

	for (int i = 0; i < 3; ++i) {
		//printf("%u act", gym->zrc.frame);
		ai_act(&gym->zrc, gym->agent, action);
		//puts(" done");
		//printf("%u update", gym->zrc.frame);
		gym_update(gym);
		//puts(" done");

		ai_t *ai = ZRC_GET(&gym->zrc, ai, gym->agent);
		if (ai) {
			*reward += ai->reward;
		} else {
			*done = 1;
			ai_t *aip = ZRC_GET_PREV(&gym->zrc, ai, gym->agent);
			printf("done %.2f\n", aip->total_reward);
			return;
		}
	}

	//printf("%u obs", gym->zrc.frame);
	ai_observe(&gym->zrc, gym->agent, observation);
	//puts(" done");
}