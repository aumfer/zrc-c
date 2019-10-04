#include <draw_locomotion.h>

static float vertices[] = {
	-1, -1,
	-1, +1,
	+1, -1,
	+1, +1
};
static uint16_t indices[] = {
	0, 1, 3,
	0, 3, 2
};

#define GLSL_DEFINE(x) "\n" x "\n"
#define GLSL_BEGIN \
	GLSL_DEFINE("#version 430")
#define GLSL(...) #__VA_ARGS__

static const char vertex_source[] = GLSL_BEGIN
GLSL(
in vec2 texcoord;

out vec2 f_texcoord;

void main() {
	f_texcoord = texcoord;
	gl_Position = vec4(texcoord, 0, 1);
});

static const char fragment_source[] = GLSL_BEGIN
#include <shaders/util.glsl>
GLSL(
uniform sampler2D potential;

in vec2 f_texcoord;

out vec4 p_color;

void main() {
	vec2 texcoord = unorm(f_texcoord);
	vec4 color = texture(potential, texcoord).rrrr;
	p_color = color;
});

void draw_locomotion_create(draw_locomotion_t *draw_locomotion) {
	draw_locomotion->image = sg_make_image(&(sg_image_desc) {
		.width = DRAW_LOCOMOTION_SIZE,
		.height = DRAW_LOCOMOTION_SIZE,
		.usage = SG_USAGE_STREAM,
		.pixel_format = SG_PIXELFORMAT_R8,
		.min_filter = SG_FILTER_LINEAR,
		.mag_filter = SG_FILTER_LINEAR
	});

	draw_locomotion->vertex_buffer = sg_make_buffer(&(sg_buffer_desc) {
		.size = sizeof(vertices),
		.content = vertices
	});

	draw_locomotion->index_buffer = sg_make_buffer(&(sg_buffer_desc) {
		.size = sizeof(indices),
		.type = SG_BUFFERTYPE_INDEXBUFFER,
		.content = indices
	});

	draw_locomotion->program = sg_make_pipeline(&(sg_pipeline_desc) {
		.layout = {
			.attrs = {
				[0].format = SG_VERTEXFORMAT_FLOAT2
			}
		},
		.shader = sg_make_shader(&(sg_shader_desc) {
			.attrs = {
				[0].name = "texcoord"
			},
			.vs.source = vertex_source,
			.fs.source = fragment_source,
			.fs.images = {
				[0].name = "potential",
				[0].type = SG_IMAGETYPE_2D
			}
		}),
		.index_type = SG_INDEXTYPE_UINT16,
		.blend = {
			.enabled = true,
			.src_factor_rgb = SG_BLENDFACTOR_ONE,
			.src_factor_alpha = SG_BLENDFACTOR_ONE,
			.dst_factor_rgb = SG_BLENDFACTOR_ONE,
			.dst_factor_alpha = SG_BLENDFACTOR_ONE,
		}
	});
}
void draw_locomotion_delete(draw_locomotion_t *draw_locomotion) {

}

void draw_locomotion_frame(draw_locomotion_t *draw_locomotion, const zrc_t *zrc, const camera_t *camera, const control_t *control, float dt) {
	draw_locomotion->accumulator += dt;
	if (draw_locomotion->accumulator < DRAW_LOCOMOTION_RATE) {
		return;
	}
	draw_locomotion->accumulator = 0;
	id_t id = control->unit;
	const locomotion_t *locomotion = ZRC_GET(zrc, locomotion, id);
	const physics_t *physics = ZRC_GET(zrc, physics, id);
	if (!locomotion || !physics) {
		return;
	}
	if (!locomotion->num_behaviors) {
		return;
	}
	float viewport[4] = { 0, 0, DRAW_LOCOMOTION_SIZE, DRAW_LOCOMOTION_SIZE };
	double minp = FLT_MAX;
	double maxp = -FLT_MAX;
	for (int y = 0; y < DRAW_LOCOMOTION_SIZE; ++y) {
		for (int x = 0; x < DRAW_LOCOMOTION_SIZE; ++x) {
			hmm_vec3 ro = hmm_unproject(HMM_Vec3((float)x, (float)y, 0), camera->view_projection, viewport);
			hmm_vec3 end = hmm_unproject(HMM_Vec3((float)x, (float)y, 1), camera->view_projection, viewport);
			hmm_vec3 rd = HMM_NormalizeVec3(HMM_SubtractVec3(ro, end));
			float t = isect_plane(ro, rd, HMM_Vec4(0, 0, 1, 0));
			hmm_vec3 hit = HMM_AddVec3(ro, HMM_MultiplyVec3f(rd, t));

			double potential = 0;
			for (int i = 0; i < locomotion->num_behaviors; ++i) {
				const locomotion_behavior_t *behavior = &locomotion->behaviors[i];
				cpVect point = cpv(hit.X, hit.Y);
				cpVect offset = cpvsub(point, physics->position);
				cpVect direction = cpvnormalize(offset);
				float angle = cpvtoangle(direction);
				double p = (*behavior)(zrc, id, point, angle);
				potential += p;
			}
			draw_locomotion->potential[y][x] = (float)potential;
			minp = min(minp, potential);
			maxp = max(maxp, potential);
		}
	}

	if (minp == maxp) {
		return;
	}

	for (int x = 0; x < DRAW_LOCOMOTION_SIZE; ++x) {
		for (int y = 0; y < DRAW_LOCOMOTION_SIZE; ++y) {
			double norm = (draw_locomotion->potential[x][y] - minp) / (maxp - minp);
			draw_locomotion->scaled_potential[x][y] = (uint8_t)(norm * 255);
		}
	}

	sg_update_image(draw_locomotion->image, &(sg_image_content) {
		.subimage[0][0] = {
			.size = sizeof(draw_locomotion->scaled_potential),
			.ptr = draw_locomotion->scaled_potential
		}
	});

	sg_begin_default_pass(&(sg_pass_action) {
		.colors[0].action = SG_ACTION_DONTCARE,
		.depth.action = SG_ACTION_DONTCARE,
		.stencil.action = SG_ACTION_DONTCARE
	}, sapp_width(), sapp_height());

	sg_apply_pipeline(draw_locomotion->program);
	sg_apply_bindings(&(sg_bindings) {
		.vertex_buffers = {
			[0] = draw_locomotion->vertex_buffer
		},
		.index_buffer = draw_locomotion->index_buffer,
		.fs_images[0] = draw_locomotion->image
	});

	int num_elements = _countof(indices);
	sg_draw(0, num_elements, 1);

	sg_end_pass();
	sg_commit();
}