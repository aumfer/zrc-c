#include <draw_world.h>
#include <HandmadeMath.h>
#include <stdlib.h>
#include <sokol_app.h>

typedef struct vertex {
	float texcoord[2];
} vertex_t;

typedef uint32_t index_t;

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
#include <shaders/util.glsl>
GLSL(
uniform mat4 view_projection;

in vec2 texcoord;

out vec2 f_position;

void main() {
	mat4 transform = mat4_translate(1024*4, 1024*4, 0) * mat4_scale(1024*8, 1024*8, 0);
	vec4 p = transform * vec4(texcoord, 0, 1);
	f_position = p.xy;
	gl_Position = view_projection * p;
});

static const char fragment_source[] = GLSL_BEGIN
#include <shaders/util.glsl>
#include <shaders/sdf.glsl>
GLSL(
in vec2 f_position;

out vec4 p_color;

void main() {
	vec3 color = vec3(0);

	float d = 1e10;
	d = min(d, grid_line((f_position) / 16) * 16);
	//d = min(d, grid_point((v_position + map_scale / 2) / map_scale) * map_scale);
	//d = min(d, hexagon(v_position, map_scale).z);

	//color += rgb(3, 3, 3);
	color += vec3(hash1(f_position)) * 0.01;
	color += rgb(100, 149, 237) * fill2(d) * 0.05;
	
	color = pow(color, vec3(1.0 / 2.2));
	p_color = vec4(color, 1.0);
});

void draw_world_create(draw_world_t *draw_world) {
	draw_world->vertex_buffer = sg_make_buffer(&(sg_buffer_desc) {
		.size = sizeof(vertices),
		.content = vertices
	});

	draw_world->index_buffer = sg_make_buffer(&(sg_buffer_desc) {
		.size = sizeof(indices),
		.type = SG_BUFFERTYPE_INDEXBUFFER,
		.content = indices
	});

	draw_world->program = sg_make_pipeline(&(sg_pipeline_desc) {
		.layout = {
			.buffers = {
				[0].stride = sizeof(vertex_t)
			},
			.attrs = {
				[0].buffer_index = 0,
				[0].format = SG_VERTEXFORMAT_FLOAT2,
				[0].offset = offsetof(vertex_t, texcoord),
			}
		},
		.shader = sg_make_shader(&(sg_shader_desc) {
			.attrs = {
				[0].name = "texcoord"
			},
			.vs.source = vertex_source,
			.vs.uniform_blocks = {
				[0].size = sizeof(hmm_mat4),
				[0].uniforms = {
					[0].name = "view_projection",
					[0].type = SG_UNIFORMTYPE_MAT4
				}
			},
			.fs.source = fragment_source,
		}),
		.index_type = SG_INDEXTYPE_UINT16
	});
}
void draw_world_destroy(draw_world_t *draw_world) {

}

void draw_world_tick(draw_world_t *draw_world, const camera_t *camera) {
	sg_begin_default_pass(&(sg_pass_action) {
		.colors[0].action = SG_ACTION_DONTCARE
	}, sapp_width(), sapp_height());

	sg_apply_pipeline(draw_world->program);
	sg_apply_bindings(&(sg_bindings) {
		.vertex_buffers = {
			[0] = draw_world->vertex_buffer
		},
		.index_buffer = draw_world->index_buffer
	});
	sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &camera->view_projection, sizeof(camera->view_projection));

	int num_elements = _countof(indices);
	sg_draw(0, num_elements, 1);

	sg_end_pass();
	sg_commit();
}