#pragma once

#include <zrc.h>

#define AI_STATE_LENGTH 1
#define AI_OBSERVATION_LENGTH (AI_STATE_LENGTH*?)
#define AI_ACTION_LENGTH 13

typedef enum ai_component {
	zrc_state = 0,
	zrc_reward = 0
} ai_component_t;

typedef struct ai {
	// static
	float ability[ABILITY_COUNT][ABILITY_COUNT];

	// persistent
	float state[MAX_FRAMES][MAX_ENTITIES][AI_STATE_LENGTH];
	float reward[MAX_FRAMES][MAX_ENTITIES];
} ai_t;

void ai_startup(ai_t *);
void ai_shutdown(ai_t *);
void ai_update(zrc_t *, ai_t *);
void ai_observe(const zrc_t *, const ai_t *, id_t id, float *numpyarray);
void ai_act(zrc_t *, const ai_t *, id_t id, float *numpyarray);
