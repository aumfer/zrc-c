#include <draw_ai.h>
#include <zmath.h>
#include <string.h>

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

void draw_ai_create(draw_ai_t *draw_ai) {
	draw_ai->image = sg_make_image(&(sg_image_desc) {
		.width = RL_LIDAR,
		.height = 2 * (DRAW_AI_ENTITIES + 1),
		.usage = SG_USAGE_STREAM,
		.pixel_format = SG_PIXELFORMAT_R8
	});

	draw_ai->vertex_buffer = sg_make_buffer(&(sg_buffer_desc) {
		.size = sizeof(vertices),
		.content = vertices
	});

	draw_ai->index_buffer = sg_make_buffer(&(sg_buffer_desc) {
		.size = sizeof(indices),
		.type = SG_BUFFERTYPE_INDEXBUFFER,
		.content = indices
	});

	draw_ai->program = sg_make_pipeline(&(sg_pipeline_desc) {
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
void draw_ai_delete(draw_ai_t *draw_ai) {

}

void draw_ai_frame(draw_ai_t *draw_ai, const zrc_t *zrc, const camera_t *camera, const control_t *control, float dt) {
	id_t id = control->unit;
	const rl_t *ai = ZRC_GET(zrc, rl, id);
	if (!ai) return;

	rl_obs_t locomotion_obs;
	rl_observe(zrc, id, &locomotion_obs);

	uint8_t locomotion_lidar[(DRAW_AI_ENTITIES + 1)][2][RL_LIDAR] = { 0 };
	//memset(locomotion_lidar, 0, sizeof(locomotion_lidar));
	for (int i = 0; i < RL_LIDAR; ++i) {
		locomotion_lidar[0][0][i] = (uint8_t)(unorm(locomotion_obs.command.dist[i]) * 255);
		locomotion_lidar[0][1][i] = (uint8_t)(unorm(locomotion_obs.command.align[i]) * 255);

		//for (int j = 0; j < DRAW_AI_ENTITIES; ++j) {
		//	locomotion_lidar[j+1][0][i] = (uint8_t)(unorm(locomotion_obs.sense[j].dist[i]) * 255);
		//	locomotion_lidar[j+1][1][i] = (uint8_t)(unorm(locomotion_obs.sense[j].align[i]) * 255);
		//}
	}

	sg_update_image(draw_ai->image, &(sg_image_content) {
		.subimage[0][0] = {
			.size = sizeof(locomotion_lidar),
			.ptr = locomotion_lidar
		}
	});

	sg_begin_default_pass(&(sg_pass_action) {
		.colors[0].action = SG_ACTION_DONTCARE,
		.depth.action = SG_ACTION_DONTCARE,
		.stencil.action = SG_ACTION_DONTCARE
	}, sapp_width(), sapp_height());

	sg_apply_pipeline(draw_ai->program);
	sg_apply_bindings(&(sg_bindings) {
		.vertex_buffers = {
			[0] = draw_ai->vertex_buffer
		},
		.index_buffer = draw_ai->index_buffer,
		.fs_images[0] = draw_ai->image
	});
	sg_apply_viewport(0, 0, sapp_width(), 8 * 2 * (DRAW_AI_ENTITIES + 1), true);

	sg_draw(0, _countof(indices), 1);

	sg_end_pass();
	sg_commit();
}
