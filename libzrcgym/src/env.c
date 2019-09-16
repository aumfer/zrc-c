#include <env.h>

static gym_t *gym;

int env_observation_length(void) {
	return AI_OBSERVATION_LENGTH;
}
int env_action_length(void) {
	return AI_ACTION_LENGTH;
}

void env_reset(void) {
	if (gym) {
		gym_delete(gym);
	}
	gym = calloc(1, sizeof(gym_t));
	gym_create(gym);
}

void env_step(float *action, float *observation, float *reward, int *done) {
	if (!gym) return;

	for (int i = 0; i < 3; ++i) {
		ai_act(&gym->zrc, gym->agent, action);
		gym_update(gym);
		ai_t *ai = ZRC_GET(&gym->zrc, ai, gym->agent);
		if (ai) {
			*reward += ai->reward;
		} else {
			*done = 1;
			return;
		}
	}

	ai_observe(&gym->zrc, gym->agent, observation);

	*done = 0;
}