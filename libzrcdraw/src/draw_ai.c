#include <draw_ai.h>
#include <zmath.h>

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
	draw_ai->locomotion_image = sg_make_image(&(sg_image_desc) {
		.width = AI_LIDAR,
		.height = AI_LOCOMOTION_ENTITY_LENGTH,
		.usage = SG_USAGE_STREAM,
		.pixel_format = SG_PIXELFORMAT_R8
	});
	draw_ai->sense_image = sg_make_image(&(sg_image_desc) {
		.width = AI_LIDAR,
			.height = AI_SENSE_ENTITY_LENGTH,
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
	const ai_t *ai = ZRC_GET(zrc, ai, id);
	if (!ai) return;

	uint8_t locomotion_lidar[AI_LIDAR*AI_LOCOMOTION_ENTITY_LENGTH];
	for (int i = 0; i < AI_LIDAR*AI_LOCOMOTION_ENTITY_LENGTH; i += AI_LOCOMOTION_ENTITY_LENGTH) {
		locomotion_lidar[i+0] = (uint8_t)(unorm(ai->locomotion_obs[i+0]) * 255);
		locomotion_lidar[i+1] = (uint8_t)(unorm(ai->locomotion_obs[i+1]) * 255);
	}

	sg_update_image(draw_ai->locomotion_image, &(sg_image_content) {
		.subimage[0][0] = {
			.size = sizeof(locomotion_lidar),
			.ptr = locomotion_lidar
		}
	});

	uint8_t sense_lidar[AI_LIDAR*AI_SENSE_ENTITY_LENGTH];
	for (int i = 0; i < AI_LIDAR*AI_SENSE_ENTITY_LENGTH; i += AI_SENSE_ENTITY_LENGTH) {
		sense_lidar[i + 0] = (uint8_t)(unorm(ai->sense_obs[i+0]) * 255);
		sense_lidar[i + 1] = (uint8_t)(unorm(ai->sense_obs[i+1]) * 255);
		sense_lidar[i + 2] = (uint8_t)(unorm(ai->sense_obs[i+2]) * 255);
	}

	sg_update_image(draw_ai->sense_image, &(sg_image_content) {
		.subimage[0][0] = {
			.size = sizeof(sense_lidar),
			.ptr = sense_lidar
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
			.fs_images[0] = draw_ai->locomotion_image
	});
	sg_apply_viewport(0, 0, sapp_width(), 16, true);

	sg_draw(0, _countof(indices), 1);

	sg_apply_bindings(&(sg_bindings) {
		.vertex_buffers = {
			[0] = draw_ai->vertex_buffer
		},
			.index_buffer = draw_ai->index_buffer,
			.fs_images[0] = draw_ai->sense_image
	});
	sg_apply_viewport(0, 16, sapp_width(), 24, true);

	sg_draw(0, _countof(indices), 1);

	sg_end_pass();
	sg_commit();
}