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
	p_color = vec4(color.r, 0, 0, 0.5);
});

void draw_ai_create(draw_ai_t *draw_ai) {
	draw_ai->image = sg_make_image(&(sg_image_desc) {
		.width = RL_OBS_NUM_TURNS,
		.height = RL_OBS_NUM_MOVES,
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
				//.src_factor_rgb = SG_BLENDFACTOR_ONE,
				//.src_factor_alpha = SG_BLENDFACTOR_ONE,
				//.dst_factor_rgb = SG_BLENDFACTOR_ONE,
				//.dst_factor_alpha = SG_BLENDFACTOR_ONE,
		}
	});
}
void draw_ai_delete(draw_ai_t *draw_ai) {

}

void draw_ai_frame(draw_ai_t *draw_ai, const zrc_t *zrc, const camera_t *camera, const control_t *control, float dt) {
	id_t id = control->unit;
	const rl_t *ai = ZRC_GET(zrc, rl, id);
	if (!ai) return;

	const rl_obs_t *rl_obs;
	ZRC_RECEIVE(zrc, rl_obs, id, &draw_ai->recv_rl_obs, rl_obs, {});
	
	if (rl_obs) {
		uint8_t img_obs[RL_OBS_NUM_TURNS][RL_OBS_NUM_MOVES];
		for (int i = 0; i < RL_OBS_NUM_MOVES; ++i) {
			for (int j = 0; j < RL_OBS_NUM_TURNS; ++j) {
				img_obs[j][i] = (uint8_t)(unorm(rl_obs->values[i][j]) * 255);
			}
		}

		sg_update_image(draw_ai->image, &(sg_image_content) {
			.subimage[0][0] = {
				.size = sizeof(img_obs),
				.ptr = img_obs
			}
		});
	}

	sg_begin_default_pass(&(sg_pass_action) {
		.colors[0].action = SG_ACTION_LOAD,
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
	sg_apply_viewport(0, sapp_height() / 2, sapp_width(), sapp_height()/2, true);

	sg_draw(0, _countof(indices), 1);

	sg_end_pass();
	sg_commit();
}
