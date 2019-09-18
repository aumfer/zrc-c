#include <draw_blur.h>
#include <sokol_app.h>
#include <stdlib.h>

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
	uniform sampler2D source;

in vec2 f_texcoord;

out vec4 p_color;

void main() {
	vec2 uv = unorm(f_texcoord);

	vec4 color = texture2D(source, uv);

	// todo textureGather
	// gradient = direction to light
	// shade with NoL

	p_color = color;
});

void draw_blur_create(draw_blur_t *draw_blur) {
	draw_blur->vertex_buffer = sg_make_buffer(&(sg_buffer_desc) {
		.size = sizeof(vertices),
		.content = vertices
	});

	draw_blur->index_buffer = sg_make_buffer(&(sg_buffer_desc) {
		.size = sizeof(indices),
		.type = SG_BUFFERTYPE_INDEXBUFFER,
		.content = indices
	});

	draw_blur->program = sg_make_pipeline(&(sg_pipeline_desc) {
		.layout = {
			.attrs = {
				[0].buffer_index = 0,
				[0].format = SG_VERTEXFORMAT_FLOAT2
			}
		},
		.shader = sg_make_shader(&(sg_shader_desc) {
			.attrs = {
				[0].name = "texcoord",
			},
			.vs.source = vertex_source,
			.fs.source = fragment_source,
			.fs.images = {
				[0].name = "source",
				[0].type = SG_IMAGETYPE_2D
			}
		}),
		.index_type = SG_INDEXTYPE_UINT16,
		.blend = {
			.enabled = true,
			.src_factor_rgb = SG_BLENDFACTOR_ONE,
			.src_factor_alpha = SG_BLENDFACTOR_ONE,
			.dst_factor_rgb = SG_BLENDFACTOR_ONE,
			.dst_factor_alpha = SG_BLENDFACTOR_ONE
		}
	});
}
void draw_blur_delete(draw_blur_t *draw_blur) {
}

void draw_blur_draw(draw_blur_t *draw_blur, sg_image blur) {
	sg_begin_default_pass(&(sg_pass_action) {
		.colors[0].action = SG_ACTION_DONTCARE,
			.depth.action = SG_ACTION_DONTCARE,
			.stencil.action = SG_ACTION_DONTCARE
	}, sapp_width(), sapp_height());

	sg_apply_pipeline(draw_blur->program);
	sg_apply_bindings(&(sg_bindings) {
		.vertex_buffers = {
			[0] = draw_blur->vertex_buffer
		},
		.index_buffer = draw_blur->index_buffer,
		.fs_images[0] = blur
	});

	int num_elements = _countof(indices);
	sg_draw(0, num_elements, 1);

	sg_end_pass();
	sg_commit();
}