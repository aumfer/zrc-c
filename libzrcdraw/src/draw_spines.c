#include <draw_spines.h>
#include <zmath.h>
#include <sokol_app.h>

typedef struct vs_uniforms {
	hmm_mat4 view_projection;
} vs_uniforms_t;

typedef struct fs_uniforms {
	hmm_vec2 camera_position;
} fs_uniforms_t;

#define GLSL_DEFINE(x) "\n" x "\n"
#define GLSL_BEGIN \
	GLSL_DEFINE("#version 430")
#define GLSL(...) #__VA_ARGS__

static const char vertex_source[] = GLSL_BEGIN
#include <shaders/util.glsl>
GLSL(
uniform mat4 view_projection;

in vec2 position;
in vec4 annotations;

out vec2 v_position;
out vec4 v_annotations;

void main() {
	v_position = position;
	v_annotations = annotations;

	vec4 p = vec4(position, 4, 1);
	gl_Position = view_projection * p;
});

static const char fragment_source[] = GLSL_BEGIN
#include <shaders/util.glsl>
#include <shaders/sdf.glsl>
GLSL(
in vec2 v_position;
in vec4 v_annotations;

out vec4 p_color;

void main() {
	vec4 color = vec4(pal(v_annotations.x, vec3(1), vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)), 1);
	//color = vec4(1, 0, 0, 1);
	color /= ((v_annotations.x)+abs(v_annotations.y));
	color *= 10;
	p_color = color;
});

void draw_spines_create(draw_spines_t *draw_spines) {
	draw_spines->program = sg_make_pipeline(&(sg_pipeline_desc) {
		.layout = {
			.buffers = {
				[0].stride = sizeof(parsl_position),
				[1].stride = sizeof(parsl_annotation)
			},
			.attrs = {
				[0].buffer_index = 0,
				[0].offset = 0,
				[0].format = SG_VERTEXFORMAT_FLOAT2,
				[1].buffer_index = 1,
				[1].offset = 0,
				[1].format = SG_VERTEXFORMAT_FLOAT4
			}
		},
		.shader = sg_make_shader(&(sg_shader_desc) {
			.attrs = {
				[0].name = "position",
				[1].name = "annotations"
			},
			.vs.source = vertex_source,
			.vs.uniform_blocks = {
				[0].size = sizeof(vs_uniforms_t),
				[0].uniforms = {
					[0].name = "view_projection",
					[0].type = SG_UNIFORMTYPE_MAT4,
				}
			},
			.fs.source = fragment_source,
			.fs.uniform_blocks = {
				[0].size = sizeof(fs_uniforms_t),
				[0].uniforms = {
					[0].name = "camera_position",
					[0].type = SG_UNIFORMTYPE_FLOAT2
				}
			},
		}),
		.index_type = SG_INDEXTYPE_UINT32,
		.blend = {
			.enabled = true,
			.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
			.src_factor_alpha = SG_BLENDFACTOR_SRC_ALPHA,
			.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
			.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
			.depth_format = SG_PIXELFORMAT_NONE
		}
	});
}
void draw_spines_delete(draw_spines_t *draw_spines) {

}

void draw_spines_draw(draw_spines_t *draw_spines, spines_t *spines, const camera_t *camera) {
	if (!spines->mesh) {
		return;
	}
	sg_begin_default_pass(&(sg_pass_action) {
		.colors[0].action = SG_ACTION_LOAD,
		.depth.action = SG_ACTION_DONTCARE,
		.stencil.action = SG_ACTION_DONTCARE
	}, sapp_width(), sapp_height());

	sg_apply_pipeline(draw_spines->program);
	sg_apply_bindings(&(sg_bindings) {
		.vertex_buffers = {
			[0] = spines->positions_buffer,
			[1] = spines->annotations_buffer
		},
		.index_buffer = spines->index_buffer
	});
	vs_uniforms_t vs_uniforms = {
		.view_projection = camera->view_projection
	};
	fs_uniforms_t fs_uniforms = {
		.camera_position = HMM_Vec2(camera->position[0], camera->position[1])
	};
	sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &vs_uniforms, sizeof(vs_uniforms));
	sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &fs_uniforms, sizeof(fs_uniforms));

	sg_draw(0, spines->mesh->num_triangles*3, 1);

	sg_end_pass();
	sg_commit();
}