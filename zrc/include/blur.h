#pragma once

#include <sokol_gfx.h>

#define BLUR_IMAGE_SIZE 1024
#define BLUR_PASSES 2

typedef struct blur {
	sg_buffer vertex_buffer, index_buffer, instance_buffer;
	sg_pipeline program;

	sg_image images[2];
	sg_pass passes[2];
} blur_t;

void blur_create(blur_t *, sg_image source);
void blur_destroy(blur_t *);

sg_image blur_draw(blur_t *);