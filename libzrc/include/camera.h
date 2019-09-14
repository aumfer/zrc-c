#pragma once

#include <zmath.h>

typedef struct camera {
	float position[2];
	float target[2];
	float zoom;

	hmm_vec3 lerp_position;
	hmm_vec3 lerp_target;

	hmm_mat4 projection;
	hmm_mat4 view;
	hmm_mat4 view_projection;
	hmm_mat4 inv_view_projection;
} camera_t;

void camera_frame(camera_t *, float dt);