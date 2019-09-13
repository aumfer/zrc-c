#include <blur.h>
#include <HandmadeMath.h>
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
uniform vec2 direction;

in vec2 f_texcoord;

out vec4 p_color;

void main() {
	vec2 uv = unorm(f_texcoord);
	const ivec2 resolution = textureSize(source, 0);

	vec4 color = vec4(0.0);
	vec2 off1 = vec2(1.411764705882353) * direction;
	vec2 off2 = vec2(3.2941176470588234) * direction;
	vec2 off3 = vec2(5.176470588235294) * direction;
	
	color += pow(texture2D(source, uv), vec4(2.2)) * 0.1964825501511404;
	color += pow(texture2D(source, uv + (off1 / resolution)), vec4(2.2)) * 0.2969069646728344;
	color += pow(texture2D(source, uv - (off1 / resolution)), vec4(2.2)) * 0.2969069646728344;
	color += pow(texture2D(source, uv + (off2 / resolution)), vec4(2.2)) * 0.09447039785044732;
	color += pow(texture2D(source, uv - (off2 / resolution)), vec4(2.2)) * 0.09447039785044732;
	color += pow(texture2D(source, uv + (off3 / resolution)), vec4(2.2)) * 0.010381362401148057;
	color += pow(texture2D(source, uv - (off3 / resolution)), vec4(2.2)) * 0.010381362401148057;

	color = pow(color, vec4(1.0 / 2.2));
	p_color = color;
});

void blur_create(blur_t *blur, sg_image image) {
	blur->vertex_buffer = sg_make_buffer(&(sg_buffer_desc) {
		.size = sizeof(vertices),
		.content = vertices
	});

	blur->index_buffer = sg_make_buffer(&(sg_buffer_desc) {
		.size = sizeof(indices),
		.type = SG_BUFFERTYPE_INDEXBUFFER,
		.content = indices
	});

	blur->program = sg_make_pipeline(&(sg_pipeline_desc) {
		.layout = {
			.attrs = {
				[0].format = SG_VERTEXFORMAT_FLOAT2
			}
		},
		.shader = sg_make_shader(&(sg_shader_desc) {
			.attrs = {
				[0].name = "texcoord",
			},
			.vs.source = vertex_source,
			.fs.source = fragment_source,
			.fs.uniform_blocks = {
				[0].size = sizeof(hmm_vec2),
				[0].uniforms = {
					[0].name = "direction",
					[0].type = SG_UNIFORMTYPE_FLOAT2
				}
			},
			.fs.images = {
				[0].name = "source",
				[0].type = SG_IMAGETYPE_2D
			}
		}),
		.index_type = SG_INDEXTYPE_UINT16,
		.blend = {
			.depth_format = SG_PIXELFORMAT_NONE
		}
	});

	blur->images[0] = sg_make_image(&(sg_image_desc) {
		.render_target = true,
		.width = BLUR_IMAGE_SIZE,
		.height = BLUR_IMAGE_SIZE,
		.min_filter = SG_FILTER_LINEAR,
		.mag_filter = SG_FILTER_LINEAR
	});
	blur->images[1] = image;

	for (int i = 0; i < 2; ++i) {
		blur->passes[i] = sg_make_pass(&(sg_pass_desc) {
			.color_attachments[0].image = blur->images[i]
		});
	}
}
void blur_destroy(blur_t *blur) {
}

sg_image blur_draw(blur_t *blur) {
	for (int i = 0; i < BLUR_PASSES; ++i) {
		{
			sg_begin_pass(blur->passes[0], &(sg_pass_action) {
				.colors[0].action = SG_ACTION_DONTCARE,
			});

			sg_apply_pipeline(blur->program);
			sg_apply_bindings(&(sg_bindings) {
				.vertex_buffers = {
					[0] = blur->vertex_buffer
				},
				.index_buffer = blur->index_buffer,
				.fs_images = {
					[0] = blur->images[1]
				}
			});

			hmm_vec2 direction = HMM_Vec2(1, 0);
			sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &direction, sizeof(direction));

			int num_elements = _countof(indices);
			sg_draw(0, num_elements, 1);

			sg_end_pass();
			sg_commit();
		}

		{
			sg_begin_pass(blur->passes[1], &(sg_pass_action) {
				.colors[0].action = SG_ACTION_DONTCARE,
			});

			sg_apply_pipeline(blur->program);
			sg_apply_bindings(&(sg_bindings) {
				.vertex_buffers = {
					[0] = blur->vertex_buffer
				},
					.index_buffer = blur->index_buffer,
					.fs_images = {
						[0] = blur->images[0]
				}
			});
			hmm_vec2 direction = HMM_Vec2(0, 1);
			sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &direction, sizeof(direction));

			int num_elements = _countof(indices);
			sg_draw(0, num_elements, 1);

			sg_end_pass();
			sg_commit();
		}
	}

	return blur->images[1];
}