#define PAR_STREAMLINES_IMPLEMENTATION
#define __attribute__(x) 
#pragma warning(push)
#pragma warning(disable:4244)
#include <par_streamlines.h>
#pragma warning(pop)

#include <spines.h>

#define SPINES_MAX_SPINES (16384*16)
#define SPINES_MAX_VERTICES (SPINES_MAX_SPINES*16)

#define SPINES_POSITIONS_COUNT SPINES_MAX_VERTICES
#define SPINES_POSITIONS_SIZE (SPINES_POSITIONS_COUNT*sizeof(parsl_position))

#define SPINES_ANNOTATIONS_COUNT SPINES_MAX_VERTICES
#define SPINES_ANNOTATIONS_SIZE (SPINES_ANNOTATIONS_COUNT*sizeof(parsl_annotation))

#define SPINES_INDEX_COUNT SPINES_MAX_VERTICES
#define SPINES_INDEX_SIZE (SPINES_INDEX_COUNT*sizeof(uint32_t))

void spines_create(spines_t *spines) {
	spines->positions_buffer = sg_make_buffer(&(sg_buffer_desc) {
		.size = SPINES_POSITIONS_SIZE,
		.usage = SG_USAGE_STREAM
	});

	spines->annotations_buffer = sg_make_buffer(&(sg_buffer_desc) {
		.size = SPINES_ANNOTATIONS_SIZE,
		.usage = SG_USAGE_STREAM
	});

	spines->index_buffer = sg_make_buffer(&(sg_buffer_desc) {
		.size = SPINES_INDEX_SIZE,
		.type = SG_BUFFERTYPE_INDEXBUFFER,
		.usage = SG_USAGE_STREAM
	});

	spines->context = parsl_create_context((parsl_config) {
		.thickness = 1,
		.flags = PARSL_FLAG_ANNOTATIONS,
		.u_mode = PAR_U_MODE_DISTANCE,
		//.streamlines_seed_spacing = MAP_SCALE,
		//.streamlines_seed_viewport.right = WORLD_SIZE,
		//.streamlines_seed_viewport.top = WORLD_SIZE
	});

	spines->spine_list.vertices = (parsl_position *)calloc(SPINES_MAX_VERTICES, sizeof(parsl_position));
	spines->spine_list.spine_lengths = (uint16_t *)calloc(SPINES_MAX_SPINES, sizeof(uint16_t));
}
void spines_destroy(spines_t *spines) {

}

void spines_update(spines_t *spines, zrc_t *zrc) {
	spines->spine_list.num_spines = 0;
	spines->spine_list.num_vertices = 0;
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		if (ZRC_HAS(zrc, physics, i)) {
			int num_vertices = 0;
			for (int j = 0; j < MAX_FRAMES-1; ++j) {
				if (ZRC_HAD_PAST(zrc, physics, i, j)) {
					physics_t *physics = ZRC_GET_PAST(zrc, physics, i, j);
					spines->spine_list.vertices[spines->spine_list.num_vertices + num_vertices].x = physics->position.x;
					spines->spine_list.vertices[spines->spine_list.num_vertices + num_vertices].y = physics->position.y;
					
					++num_vertices;
				}
			}
			if (num_vertices > 1) {
				spines->spine_list.num_vertices += num_vertices;
				spines->spine_list.spine_lengths[spines->spine_list.num_spines] = num_vertices;
				++spines->spine_list.num_spines;
			}
		}
	}

	if (spines->spine_list.num_spines) {
		parsl_mesh *mesh = parsl_mesh_from_lines(spines->context, spines->spine_list);
		spines->mesh = mesh;

		sg_update_buffer(spines->positions_buffer, mesh->positions, mesh->num_vertices * sizeof(parsl_position));
		sg_update_buffer(spines->annotations_buffer, mesh->annotations, mesh->num_vertices * sizeof(parsl_annotation));
		sg_update_buffer(spines->index_buffer, mesh->triangle_indices, mesh->num_triangles * 3 * sizeof(uint32_t));
	} else {
		//spines->mesh = 0;
	}
}