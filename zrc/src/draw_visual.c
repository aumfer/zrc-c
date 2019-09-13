#include <draw_visual.h>
#include <HandmadeMath.h>
#include <sokol_app.h>

typedef struct vertex {
	float texcoord[2];
} vertex_t;

typedef uint32_t index_t;

static_assert(sizeof(instance_t) % 16 == 0, "instance size");

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
in float radius;
in vec2 position;
in float angle;
in vec4 color;

out vec2 f_texcoord;
out flat float f_radius;
out flat vec2 f_position;
out flat vec4 f_color;

void main() {
	mat4 transform = mat4_translate(position.x, position.y, 0) * mat4_scale(radius * 2, radius * 2, 0);
	vec4 p = transform * vec4(texcoord, 0, 1);
	f_texcoord = p.xy;
	gl_Position = view_projection * p;

	f_radius = radius;
	f_position = position;
	f_color = color;
});

static const char fragment_source[] = GLSL_BEGIN
#include <shaders/util.glsl>
#include <shaders/sdf.glsl>
GLSL(
in vec2 f_texcoord;
in flat float f_radius;
in flat vec2 f_position;
in flat vec4 f_color;

out vec4 p_color;

void main() {
	float circle = sdCircle(f_texcoord - f_position, f_radius);
	vec3 color = f_color.rgb * fill2(abs(circle));
	float grid = grid_line(f_texcoord / 16) * 16;
	color += vec3(0, 0, 1) * fill2(grid);
	color = pow(color, vec3(1.0 / 2.2));
	p_color = vec4(color, 1);
}
);

void draw_visual_create(draw_visual_t *draw_visual) {
	draw_visual->vertex_buffer = sg_make_buffer(&(sg_buffer_desc) {
		.size = sizeof(vertices),
		.content = vertices
	});

	draw_visual->index_buffer = sg_make_buffer(&(sg_buffer_desc) {
		.size = sizeof(indices),
		.type = SG_BUFFERTYPE_INDEXBUFFER,
		.content = indices
	});

	draw_visual->instance_buffer = sg_make_buffer(&(sg_buffer_desc) {
		.size = INSTANCE_BUFFER_SIZE,
		.usage = SG_USAGE_STREAM
	});

	draw_visual->program = sg_make_pipeline(&(sg_pipeline_desc) {
		.layout = {
			.buffers = {
				[0].stride = sizeof(vertex_t),
				[1].stride = sizeof(instance_t),
				[1].step_func = SG_VERTEXSTEP_PER_INSTANCE
			},
			.attrs = {
				[0].buffer_index = 0,
				[0].format = SG_VERTEXFORMAT_FLOAT2,
				[0].offset = offsetof(vertex_t, texcoord),
				[1].buffer_index = 1,
				[1].format = SG_VERTEXFORMAT_FLOAT,
				[1].offset = offsetof(instance_t, radius),
				[2].buffer_index = 1,
				[2].format = SG_VERTEXFORMAT_FLOAT,
				[2].offset = offsetof(instance_t, angle),
				[3].buffer_index = 1,
				[3].format = SG_VERTEXFORMAT_FLOAT2,
				[3].offset = offsetof(instance_t, position),
				[4].buffer_index = 1,
				[4].format = SG_VERTEXFORMAT_FLOAT2,
				[4].offset = offsetof(instance_t, speed),
				[5].buffer_index = 1,
				[5].format = SG_VERTEXFORMAT_FLOAT,
				[5].offset = offsetof(instance_t, spin),
				[6].buffer_index = 1,
				[6].format = SG_VERTEXFORMAT_UBYTE4N,
				[6].offset = offsetof(instance_t, color),
				[6].buffer_index = 1,
				[6].format = SG_VERTEXFORMAT_UBYTE4N,
				[6].offset = offsetof(instance_t, color),
			}
		},
		.shader = sg_make_shader(&(sg_shader_desc) {
			.attrs = {
				[0].name = "texcoord",
				[1].name = "radius",
				[2].name = "angle",
				[3].name = "position",
				[4].name = "speed",
				[5].name = "spin",
				[6].name = "color"
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
		.index_type = SG_INDEXTYPE_UINT16,
		.blend = {
			.enabled = true,
			.src_factor_rgb = SG_BLENDFACTOR_ONE,
			.src_factor_alpha = SG_BLENDFACTOR_ONE,
			.dst_factor_rgb = SG_BLENDFACTOR_ONE,
			.dst_factor_alpha = SG_BLENDFACTOR_ONE
		}
	});

	draw_visual->glow = sg_make_pass(&(sg_pass_desc) {
		.color_attachments[0].image = sg_make_image(&(sg_image_desc) {
			.render_target = true,
			.width = 512,
			.height = 512
		}),
		.depth_stencil_attachment.image = sg_make_image(&(sg_image_desc) {
			.render_target = true,
			.width = 512,
			.height = 512,
			.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL
		})
	});
}
void draw_visual_destroy(draw_visual_t *draw_visual) {

}

void draw_visual_tick(draw_visual_t *draw_visual, zrc_t *zrc, const camera_t *camera, const control_t *control) {
	int instance_count = 0;
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		if (ZRC_HAS(zrc, visual, i) && ZRC_HAS(zrc, physics, i)) {
			visual_t *visual = ZRC_GET(zrc, visual, i);
			physics_t *physics = ZRC_GET(zrc, physics, i);

			instance_t instance = {
				.radius = physics->radius,
				.angle = physics->angle,
				.position = { [0] = physics->position[0], [1] = physics->position[1] },
				.color = visual->color
			};
			if (control->select == i) {
				instance.color = rgb(255, 0, 0);
			}
			assert(instance_count < INSTANCE_BUFFER_MAX);
			//sg_append_buffer(draw_visual->instance_buffer, &instance, sizeof(instance));
			draw_visual->instances[instance_count] = instance;
			++instance_count;
		}
	}

	sg_update_buffer(draw_visual->instance_buffer, draw_visual->instances, instance_count * sizeof(instance_t));

	{
		sg_begin_pass(draw_visual->glow, &(sg_pass_action) {
			.colors[0].action = SG_ACTION_CLEAR,
			.colors[0].val = { 0, 0, 0, 1 }
		});

		sg_apply_pipeline(draw_visual->program);
		sg_apply_bindings(&(sg_bindings) {
			.vertex_buffers = {
				[0] = draw_visual->vertex_buffer,
				[1] = draw_visual->instance_buffer
			},
			.index_buffer = draw_visual->index_buffer
		});
		sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &camera->view_projection, sizeof(camera->view_projection));

		int num_elements = _countof(indices);
		sg_draw(0, num_elements, instance_count);

		sg_end_pass();
		sg_commit();
	}

	sg_begin_default_pass(&(sg_pass_action) {
		.colors[0].action = SG_ACTION_DONTCARE
	}, sapp_width(), sapp_height());

	sg_apply_pipeline(draw_visual->program);
	sg_apply_bindings(&(sg_bindings) {
		.vertex_buffers = {
			[0] = draw_visual->vertex_buffer,
			[1] = draw_visual->instance_buffer
		},
		.index_buffer = draw_visual->index_buffer
	});
	sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &camera->view_projection, sizeof(camera->view_projection));

	int num_elements = _countof(indices);
	sg_draw(0, num_elements, instance_count);

	sg_end_pass();
	sg_commit();
}
