#ifndef _FONT_H_
#define _FONT_H_

#include <sokol_gfx.h>

typedef enum font_style {
	FONT_ARIAL_16,
	FONT_CONSOLAS_16,
	FONT_COUNT
} font_style_t;

typedef struct texture_font texture_font_t;

typedef struct font_vertex {
	uint32_t color;
	uint16_t texcoord[2];
	float position[2];
	//uint16_t _;
} font_vertex_t;
typedef uint32_t font_index_t;

#define GLYPH_VERTICES 4
#define GLYPH_INDICES 6
#define MAX_GLYPHS (1024*128)

#define ARRAY_BUFFER_MAX ((MAX_GLYPHS)*(GLYPH_VERTICES))
#define ARRAY_BUFFER_SIZE (sizeof(font_vertex_t)*(ARRAY_BUFFER_MAX))

#define ELEMENT_ARRAY_BUFFER_MAX ((MAX_GLYPHS)*(GLYPH_INDICES))
#define ELEMENT_ARRAY_BUFFER_SIZE (sizeof(font_index_t)*(ELEMENT_ARRAY_BUFFER_MAX))

typedef struct font {
	font_style_t style;

	sg_image texture;
	sg_buffer array_buffer, element_array_buffer;
	sg_pipeline program;

	font_vertex_t vertices[ARRAY_BUFFER_MAX];
	font_index_t indices[ELEMENT_ARRAY_BUFFER_MAX];

	unsigned glyph_count;
} font_t;

void font_create(font_t *, font_style_t);
void font_destroy(font_t *);

void font_begin(font_t *);
void font_print(font_t *, const char *, const float position[2], const uint32_t color);
void font_draw(font_t *);
void font_end(font_t *);


#endif