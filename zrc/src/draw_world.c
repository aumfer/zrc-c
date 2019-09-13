#include <draw_world.h>
#include <HandmadeMath.h>
#include <stdlib.h>
#include <sokol_app.h>
#include <string.h>

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

typedef struct vs_uniforms {
	float view_projection[4][4];
	float world_size[2];
} vs_uniforms_t;

typedef struct fs_uniforms {
	float camera_position[3];
	float map_scale;
} fs_uniforms_t;

#define GLSL_DEFINE(x) "\n" x "\n"
#define GLSL_BEGIN \
	GLSL_DEFINE("#version 430")
#define GLSL(...) #__VA_ARGS__

static const char vertex_source[] = GLSL_BEGIN
#include <shaders/util.glsl>
GLSL(
uniform mat4 view_projection;
uniform vec2 world_size;

in vec2 texcoord;

out vec2 f_position;

void main() {
	mat4 transform = mat4_translate(world_size.x/2, world_size.y/2, 0) * mat4_scale(world_size.x, world_size.y, 0);
	vec4 p = transform * vec4(texcoord, 0, 1);
	f_position = p.xy;
	gl_Position = view_projection * p;
});

static const char fragment_source[] = GLSL_BEGIN
#include <shaders/util.glsl>
#include <shaders/sdf.glsl>
GLSL(
uniform vec3 camera_position;
uniform float map_scale;

in vec2 f_position;

out vec4 p_color;

void main() {
	vec3 color = vec3(0);

	vec3 sqp = vec3(f_position, 0);
	vec3 rd = normalize(sqp - camera_position);
	float hext = isect_plane(sqp, rd, vec4(0,0,1,16));
	vec3 hexp = sqp + rd * hext;

	color += rgb(1, 1, 1);
	color += vec3(hash1(f_position)) * 0.001;
	color += rgb(100, 149, 237) * fill2(grid_line((f_position) / map_scale) * map_scale) * 0.05;
	color += rgb(100, 149, 237) * fill2(hexagon(hexp.xy, map_scale*4).z / 4) * 0.01;
	
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
				[0].size = sizeof(vs_uniforms_t),
				[0].uniforms = {
					[0].name = "view_projection",
					[0].type = SG_UNIFORMTYPE_MAT4,
					[1].name = "world_size",
					[1].type = SG_UNIFORMTYPE_FLOAT2
				}
			},
			.fs.source = fragment_source,
			.fs.uniform_blocks = {
				[0].size = sizeof(fs_uniforms_t),
				[0].uniforms = {
					[0].name = "camera_position",
					[0].type = SG_UNIFORMTYPE_FLOAT3,
					[1].name = "map_scale",
					[1].type = SG_UNIFORMTYPE_FLOAT
				}
			},
		}),
		.index_type = SG_INDEXTYPE_UINT16
	});
}
void draw_world_destroy(draw_world_t *draw_world) {

}

void draw_world_tick(draw_world_t *draw_world, const camera_t *camera) {
	sg_begin_default_pass(&(sg_pass_action) {
		0
	}, sapp_width(), sapp_height());

	sg_apply_pipeline(draw_world->program);
	sg_apply_bindings(&(sg_bindings) {
		.vertex_buffers = {
			[0] = draw_world->vertex_buffer
		},
		.index_buffer = draw_world->index_buffer
	});
	vs_uniforms_t vs_uniforms = {
		//.view_projection = camera->view_projection,
		.world_size = { 16384, 16384 }
	};
	memcpy(&vs_uniforms.view_projection, &camera->view_projection, sizeof(vs_uniforms.view_projection));
	fs_uniforms_t fs_uniforms = {
		.camera_position = { camera->position[0], camera->position[1], camera->zoom },
		.map_scale = 16
	};
	sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &vs_uniforms, sizeof(vs_uniforms));
	sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &fs_uniforms, sizeof(fs_uniforms));

	int num_elements = _countof(indices);
	sg_draw(0, num_elements, 1);

	sg_end_pass();
	sg_commit();
}