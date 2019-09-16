#pragma once

#include <zrc.h>
#include <zrc_host.h>
#include <ai.h>

typedef struct gym {
	zrc_t zrc;
	zrc_host_t zrc_host;
	ai_t ai;
	id_t agent;
} gym_t;

void gym_create(gym_t *);
void gym_delete(gym_t *);
void gym_update(gym_t *);