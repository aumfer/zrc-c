#include <font.h>
#include <fonts/arial-16.h>
#include <fonts/consolas-16.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <HandmadeMath.h>
#include <sokol_app.h>

#define FONT_TEXTURE_UNIT 0

static_assert(ARRAY_BUFFER_SIZE % 16 == 0, "array buffer size");
static_assert(ELEMENT_ARRAY_BUFFER_SIZE % 16 == 0, "element array buffer size");

static texture_font_t *font_data[] = {
	&font_arial16,
	&font_consolas16
};

#define GLSL_DEFINE(x) "\n" x "\n"
#define GLSL_BEGIN \
	GLSL_DEFINE("#version 430")
#define GLSL(...) #__VA_ARGS__

static const char vertex_source[] = GLSL_BEGIN GLSL(
uniform mat4 projection;

in vec4 color;
in vec2 texcoord;
in vec2 position;

out vec4 f_color;
out vec2 f_texcoord;

void main() {
	f_color = color;
	f_texcoord = texcoord;
	gl_Position = projection * vec4(position, 0, 1);
}
);
static const char fragment_source[] = GLSL_BEGIN GLSL(
uniform sampler2D font;

in vec4 f_color;
in vec2 f_texcoord;

out vec4 p_color;

void main() {
	p_color = f_color * texture(font, f_texcoord).rrrr;
}
);

void font_create(font_t *font, font_style_t style) {
	font->style = style;

	font->texture = sg_make_image(&(sg_image_desc) {
		.type = SG_IMAGETYPE_2D,
		.width = (int)font_data[style]->tex_width,
		.height = (int)font_data[style]->tex_height,
		.pixel_format = SG_PIXELFORMAT_R8,
		.min_filter = SG_FILTER_LINEAR,
		.mag_filter = SG_FILTER_LINEAR,
		.wrap_u = SG_WRAP_CLAMP_TO_EDGE,
		.wrap_v = SG_WRAP_CLAMP_TO_EDGE,
		.content.subimage[0][0].ptr = font_data[style]->tex_data,
		.content.subimage[0][0].size = sizeof(font_data[style]->tex_data)
	});

	font->array_buffer = sg_make_buffer(&(sg_buffer_desc) {
		.size = ARRAY_BUFFER_SIZE,
		.usage = SG_USAGE_STREAM
	});

	font->element_array_buffer = sg_make_buffer(&(sg_buffer_desc) {
		.size = ELEMENT_ARRAY_BUFFER_SIZE,
		.type = SG_BUFFERTYPE_INDEXBUFFER,
		.usage = SG_USAGE_STREAM
	});

	font->program = sg_make_pipeline(&(sg_pipeline_desc) {
		.layout = {
			.buffers = {
				[0].stride = sizeof(font_vertex_t)
			},
			.attrs = {
				[0].buffer_index = 0,
				[0].offset = offsetof(font_vertex_t, color),
				[0].format = SG_VERTEXFORMAT_UBYTE4N,
				[1].buffer_index = 0,
				[1].offset = offsetof(font_vertex_t, texcoord),
				[1].format = SG_VERTEXFORMAT_USHORT2N,
				[2].buffer_index = 0,
				[2].offset = offsetof(font_vertex_t, position),
				[2].format = SG_VERTEXFORMAT_FLOAT2,
			}
		},
		.shader = sg_make_shader(&(sg_shader_desc) {
			.attrs = {
				[0].name = "color",
				[1].name = "texcoord",
				[2].name = "position"
			},
			.vs.source = vertex_source,
			.vs.uniform_blocks = {
				[0].size = sizeof(hmm_mat4),
				[0].uniforms = {
					[0].name = "projection",
					[0].type = SG_UNIFORMTYPE_MAT4
				}
			},
			.fs.source = fragment_source,
			.fs.images = {
				[0].name ="font",
				[0].type = SG_IMAGETYPE_2D
			}
		}),
		.index_type = SG_INDEXTYPE_UINT32,
		.blend = {
			.enabled = true,
			.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
			.src_factor_alpha = SG_BLENDFACTOR_SRC_ALPHA,
			.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
			.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA
		}
	});
}
void font_destroy(font_t *font) {

}

void font_begin(font_t *font) {
	font->glyph_count = 0;
}

void font_print(font_t *font, const char *text, const float position[2], const uint32_t color) {
	size_t print_count;
	float pen[2];
	float x[2], y[2];
	size_t i;

	pen[0] = position[0];
	pen[1] = position[1];

	print_count = strnlen_s(text, MAX_GLYPHS - font->glyph_count);
	assert(font->glyph_count + print_count < MAX_GLYPHS);

	texture_font_t *data = font_data[font->style];

	for (i = 0; i < print_count; ++i) {
		const texture_glyph_t *glyph = NULL;
		const texture_glyph_t *g;

		for (g = data->glyphs; g < data->glyphs + data->glyphs_count; ++g) {
			if (g->charcode == text[i]) {
				glyph = g;
				break;
			}
		}
		if (!glyph) {
			fprintf(stderr, "font missing glyph %c", text[i]);
			continue;
		}

		if (i > 0) {
			const kerning_t *k;
			for (k = glyph->kerning; k < glyph->kerning + glyph->kerning_count; ++k) {
				if (k->charcode == text[i - 1]) {
					pen[0] += k->kerning;
				}
			}
		}

		x[0] = pen[0] + glyph->offset_x;
		y[0] = pen[1] + glyph->offset_y;
		x[1] = x[0] + glyph->width;
		y[1] = y[0] - glyph->height;

		font_vertex_t *vertex = &font->vertices[font->glyph_count*GLYPH_VERTICES];
		font_index_t *index = &font->indices[font->glyph_count*GLYPH_INDICES];

		vertex[0].color = color;
		vertex[0].texcoord[0] = (uint16_t)(glyph->s0 * UINT16_MAX);
		vertex[0].texcoord[1] = (uint16_t)(glyph->t0 * UINT16_MAX);
		vertex[0].position[0] = x[0];
		vertex[0].position[1] = y[0];

		vertex[1].color = color;
		vertex[1].texcoord[0] = (uint16_t)(glyph->s0 * UINT16_MAX);
		vertex[1].texcoord[1] = (uint16_t)(glyph->t1 * UINT16_MAX);
		vertex[1].position[0] = x[0];
		vertex[1].position[1] = y[1];

		vertex[2].color = color;
		vertex[2].texcoord[0] = (uint16_t)(glyph->s1 * UINT16_MAX);
		vertex[2].texcoord[1] = (uint16_t)(glyph->t1 * UINT16_MAX);
		vertex[2].position[0] = x[1];
		vertex[2].position[1] = y[1];

		vertex[3].color = color;
		vertex[3].texcoord[0] = (uint16_t)(glyph->s1 * UINT16_MAX);
		vertex[3].texcoord[1] = (uint16_t)(glyph->t0 * UINT16_MAX);
		vertex[3].position[0] = x[1];
		vertex[3].position[1] = y[0];

		index[0] = font->glyph_count * GLYPH_VERTICES + 0;
		index[1] = font->glyph_count * GLYPH_VERTICES + 1;
		index[2] = font->glyph_count * GLYPH_VERTICES + 2;
		index[3] = font->glyph_count * GLYPH_VERTICES + 0;
		index[4] = font->glyph_count * GLYPH_VERTICES + 2;
		index[5] = font->glyph_count * GLYPH_VERTICES + 3;

		pen[0] += glyph->advance_x;
		pen[1] += glyph->advance_y;

		++font->glyph_count;

		//sg_append_buffer(font->array_buffer, vertex, sizeof(vertex));
		//sg_append_buffer(font->element_array_buffer, index, sizeof(index));
	}
}

void font_end(font_t *font) {
}

void font_draw(font_t *font) {
	sg_update_buffer(font->array_buffer, font->vertices, font->glyph_count*GLYPH_VERTICES * sizeof(font_vertex_t));
	sg_update_buffer(font->element_array_buffer, font->indices, font->glyph_count*GLYPH_INDICES * sizeof(font_index_t));

	hmm_mat4 projection = HMM_Orthographic(0.0f, (float)sapp_width(), 0.0f, (float)sapp_height(), 0.0f, 1.0f);

	sg_begin_default_pass(&(sg_pass_action) {
		.colors[0].action = SG_ACTION_LOAD
		//.colors[0].action = SG_ACTION_DONTCARE
	}, sapp_width(), sapp_height());

	sg_apply_pipeline(font->program);
	sg_apply_bindings(&(sg_bindings) {
		.vertex_buffers[0] = font->array_buffer,
		.index_buffer = font->element_array_buffer,
		.fs_images[0] = font->texture
	});
	sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &projection, sizeof(projection));

	sg_draw(0, font->glyph_count*GLYPH_INDICES, 1);

	sg_end_pass();
	sg_commit();
}