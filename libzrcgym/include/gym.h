#pragma once

#include <zrc.h>
#include <zrc_host.h>

typedef struct gym {
	zrc_t zrc;
	zrc_host_t zrc_host;
	id_t agent;
	recv_t recv_obs;
} gym_t;

void gym_create(gym_t *);
void gym_delete(gym_t *);
void gym_update(gym_t *);