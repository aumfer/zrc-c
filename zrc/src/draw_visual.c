#include <draw_visual.h>
#include <zmath.h>
#include <sokol_app.h>
#include <string.h>

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

typedef struct vs_uniforms {
	float view_projection[4][4];
	float offset[3];
} vs_uniforms_t;

typedef struct fs_uniforms {
	hmm_vec3 camera_position;
} fs_uniforms_t;

#define GLSL_DEFINE(x) "\n" x "\n"
#define GLSL_BEGIN \
	GLSL_DEFINE("#version 430")
#define GLSL(...) #__VA_ARGS__

static const char vertex_source[] = GLSL_BEGIN
#include <shaders/util.glsl>
GLSL(
uniform mat4 view_projection;
uniform vec3 offset;

in vec2 texcoord;
in float radius;
in vec2 position;
in float angle;
in vec4 color;
in float flags;
in vec3 life;
in vec4 target;

out vec2 f_texcoord;
out flat float f_radius;
out flat vec2 f_position;
out flat float f_angle;
out flat vec4 f_color;
out flat uint f_flags;
out flat vec3 f_life;
out flat vec4 f_target;

void main() {
	mat4 transform = mat4_translate(position.x + offset.x, position.y + offset.y, offset.z) /** mat4_rotate(f_angle, vec3(0, 0, 1))*/ * mat4_scale(radius * 2, radius * 2, 0);
	vec4 p = transform * vec4(texcoord, 0, 1);
	f_texcoord = p.xy;
	gl_Position = view_projection * p;

	f_radius = radius;
	f_position = position;
	f_angle = angle;
	f_color = color;
	f_flags = floatBitsToUint(flags);
	f_life = life;
	f_target = target;
});

static const char fragment_source[] = GLSL_BEGIN
#include <shaders/util.glsl>
#include <shaders/sdf.glsl>
GLSL(
uniform vec3 camera_position;

in vec2 f_texcoord;
in flat float f_radius;
in flat vec2 f_position;
in flat float f_angle;
in flat vec4 f_color;
in flat uint f_flags;
in flat vec3 f_life;
in flat vec4 f_target;

out vec4 p_color;

vec2 p;

vec4 life(vec4 col) {
	float w = 0.25;
	float circle = abs(sdSemiCircle(p, f_radius, M_PI, w));
	//col = mix(col, vec4(rgb(0x12, 0x12, 0x12), 0.1), fill(circle, w));
	float health = abs(sdSemiCircle(p, f_radius - 1*w, f_life.x*M_PI, w));
	col = mix(col, vec4(rgb(0xbb, 0x86, 0xfc), 1.0), fill(health, w));
	float rage = abs(sdSemiCircle(p, f_radius - 2*w, f_life.z*M_PI, w));
	col = mix(col, vec4(rgb(0x37, 0x00, 0xb3), 1.0), fill(rage, w));
	float mana = abs(sdSemiCircle(p, f_radius - 3*w, f_life.y*M_PI, w));
	col = mix(col, vec4(rgb(0x03, 0xda, 0xc6), 1.0), fill(mana, w));
	return col;
}

void main() {
	p = rotateZ(f_texcoord - f_position, -f_angle);
	vec2 target = f_target.xy - f_position;
	float atarget = atan(target.y, target.x);
	vec2 tp = rotateZ(f_texcoord - f_position, atarget);

	//float circle = abs(sdCircle(p, f_radius));
	float circle = abs(sdSemiCircle(p, f_radius, f_life.x * M_PI, 0.5));
	circle = min(circle, abs(sdSemiCircle(p, f_radius - 0.5, f_life.y * M_PI, 0.5)));
	circle = min(circle, abs(sdSemiCircle(p, f_radius - 1.0, f_life.z * M_PI, 0.5)));
	float triangle = abs(sdTriangle(tp, f_radius / 2, f_radius));
	triangle = min(triangle, abs(sdTriangle(p, f_radius)));
	float shape = min(circle, triangle);
	//shape *= 4;
	//shape /= f_radius;
	vec4 color = f_color * fill2(shape);

	color = life(color);

	if ((f_flags & 1) == 1) {
		color += vec4(1, 0, 0, 0.1) / (circle*circle);
	}

	color = pow(color, vec4(1.0 / 2.2));
	p_color = color;
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
				[7].buffer_index = 1,
				[7].format = SG_VERTEXFORMAT_FLOAT,
				[7].offset = offsetof(instance_t, flags),
				[8].buffer_index = 1,
				[8].format = SG_VERTEXFORMAT_FLOAT3,
				[8].offset = offsetof(instance_t, life),
				[9].buffer_index = 1,
				[9].format = SG_VERTEXFORMAT_FLOAT4,
				[9].offset = offsetof(instance_t, target),
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
				[6].name = "color",
				[7].name = "flags",
				[8].name = "life",
				[9].name = "target"
			},
			.vs.source = vertex_source,
			.vs.uniform_blocks = {
				[0].size = sizeof(vs_uniforms_t),
				[0].uniforms = {
					[0].name = "view_projection",
					[0].type = SG_UNIFORMTYPE_MAT4,
					[1].name = "offset",
					[1].type = SG_UNIFORMTYPE_FLOAT3
				}
			},
			.fs.source = fragment_source,
			.fs.uniform_blocks = {
			[0].size = sizeof(fs_uniforms_t),
			[0].uniforms = {
				[0].name = "camera_position",
				[0].type = SG_UNIFORMTYPE_FLOAT3
			}
			},
		}),
		.index_type = SG_INDEXTYPE_UINT16,
		.blend = {
			.enabled = true,
			.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
			.src_factor_alpha = SG_BLENDFACTOR_SRC_ALPHA,
			.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
			.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
			.depth_format = SG_PIXELFORMAT_NONE
		}
	});

	draw_visual->output = sg_make_image(&(sg_image_desc) {
		.render_target = true,
		.width = BLUR_IMAGE_SIZE,
		.height = BLUR_IMAGE_SIZE,
		.min_filter = SG_FILTER_LINEAR,
		.mag_filter = SG_FILTER_LINEAR
	});

	draw_visual->pass = sg_make_pass(&(sg_pass_desc) {
		.color_attachments[0].image = draw_visual->output
	});

	blur_create(&draw_visual->blur, draw_visual->output);
	draw_blur_create(&draw_visual->draw_blur);
}
void draw_visual_destroy(draw_visual_t *draw_visual) {

}

void draw_visual_tick(draw_visual_t *draw_visual, zrc_t *zrc, const camera_t *camera, const control_t *control) {
	float dt = (float)stm_sec(stm_since(zrc->timer.time));
	int instance_count = 0;
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		if (ZRC_HAS(zrc, visual, i) && ZRC_HAS(zrc, physics, i)) {
			assert(instance_count < INSTANCE_BUFFER_MAX);
			visual_t *visual = ZRC_GET_READ(zrc, visual, i);
			physics_t *physics = ZRC_GET_READ(zrc, physics, i);

			hmm_vec2 position = HMM_Vec2(physics->position[0], physics->position[1]);
			hmm_vec2 velocity = HMM_Vec2(physics->velocity[0], physics->velocity[1]);
			position = HMM_AddVec2(position, HMM_MultiplyVec2f(velocity, dt));

			float angle = physics->angle + physics->angular_velocity * dt;

			instance_t instance = {
				.radius = physics->radius,
				.angle = angle,
				.position = { [0] = position.X, [1] = position.Y },
				.speed = { [0] = physics->velocity[0], [1] = physics->velocity[1] },
				.spin = physics->angular_velocity,
				.color = visual->color,
				.flags = visual->flags
			};
			if (control->hover == i) {
				instance.flags |= INSTANCE_HOVER;
			}
			if (control->select == i) {
				instance.flags |= INSTANCE_SELECT;
			}
			life_t *life = ZRC_GET(zrc, life, i);
			if (life) {
				instance.life[0] = life->health / life->max_health;
				instance.life[1] = life->mana / life->max_mana;
				instance.life[2] = life->rage / life->max_rage;
			}
			caster_t *caster = ZRC_GET(zrc, caster, i);
			if (caster) {
				instance.target[0] = caster->abilities[0].target.point[0];
				instance.target[1] = caster->abilities[0].target.point[1];
				//instance.target[2] = caster->abilities[1].target.unit.position[0]
				//instance.target[3] = caster->abilities[1].target.unit.position[1]
			}
			//sg_append_buffer(draw_visual->instance_buffer, &instance, sizeof(instance));
			draw_visual->instances[instance_count] = instance;
			++instance_count;
		}
	}

	sg_update_buffer(draw_visual->instance_buffer, draw_visual->instances, instance_count * sizeof(instance_t));

	vs_uniforms_t vs_uniforms = {
		//.view_projection = camera->view_projection
		.offset = {0,0,0}
	};
	memcpy(&vs_uniforms.view_projection, &camera->view_projection, sizeof(vs_uniforms.view_projection));
	fs_uniforms_t fs_uniforms = {
		.camera_position = HMM_Vec3(camera->position[0], camera->position[1], camera->zoom)
	};
	{
		sg_begin_pass(draw_visual->pass, &(sg_pass_action) {
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
		sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &vs_uniforms, sizeof(vs_uniforms));
		sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &fs_uniforms, sizeof(fs_uniforms));

		int num_elements = _countof(indices);
		sg_draw(0, num_elements, instance_count);

		sg_end_pass();
		sg_commit();
	}

	sg_image blur = blur_draw(&draw_visual->blur);
	draw_blur_draw(&draw_visual->draw_blur, blur);
	
	vs_uniforms.offset[2] = 4;
	{
		sg_begin_default_pass(&(sg_pass_action) {
			.colors[0].action = SG_ACTION_DONTCARE,
			.depth.action = SG_ACTION_DONTCARE,
			.stencil.action = SG_ACTION_DONTCARE
		}, sapp_width(), sapp_height());

		sg_apply_pipeline(draw_visual->program);
		sg_apply_bindings(&(sg_bindings) {
			.vertex_buffers = {
				[0] = draw_visual->vertex_buffer,
				[1] = draw_visual->instance_buffer
			},
				.index_buffer = draw_visual->index_buffer
		});
		sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &vs_uniforms, sizeof(vs_uniforms));
		sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &fs_uniforms, sizeof(fs_uniforms));

		int num_elements = _countof(indices);
		sg_draw(0, num_elements, instance_count);

		sg_end_pass();
		sg_commit();
	}
}
