#pragma once

#include <sokol_gfx.h>
#include <par_streamlines.h>
#include <zrc.h>

typedef struct spines {
	sg_buffer positions_buffer, annotations_buffer;
	sg_buffer index_buffer;

	parsl_context* context;
	parsl_spine_list spine_list;

	parsl_mesh *mesh;
} spines_t;

void spines_create(spines_t *);
void spines_destroy(spines_t *);

void spines_update(spines_t *, zrc_t *);