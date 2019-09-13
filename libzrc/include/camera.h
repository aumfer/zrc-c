#pragma once

#include <HandmadeMath.h>

typedef struct camera {
	float position[2];
	float target[2];
	float zoom;

	hmm_mat4 projection;
	hmm_mat4 view;
	hmm_mat4 view_projection;
} camera_t;

void camera_update(camera_t *);