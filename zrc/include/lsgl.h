#ifndef LS_LSGL_H
#define LS_LSGL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <GL/gl3w.h>
#include <stdlib.h>

#define xstr(a) str(a)
#define str(a) #a

#define GLSL_DEFINE(x) "\n" x "\n"
#define GLSL_BEGIN \
	GLSL_DEFINE("#version 430")
#define GLSL_COMPUTE(x,y,z) \
	GLSL_DEFINE("layout(local_size_x = " xstr(x) ", local_size_y = " xstr(y) ", local_size_z = " xstr(z) ") in;")
#define GLSL_CONST(type, name) \
	GLSL_DEFINE("const " #type " " #name " = " xstr(name) ";")
#define GLSL_DEFINE_CONST(x) GLSL_DEFINE("#define " #x " " xstr(x))
#define GLSL(...) #__VA_ARGS__
#define lsgl_offsetof(s,m) ((const GLvoid *)offsetof(s,m))
#define lsgl_offsetof_off(s,m,o) ((const GLvoid *)(offsetof(s,m)+(o)))
	//#define lsgl_countof(x) _countof(x)
#define lsgl_countof(x) (sizeof(x) / sizeof(x[0]))

#ifdef _WIN32
#  define lsgl_align(...) __declspec(align(__VA_ARGS__))
#else
#  define lsgl_align(...) __attribute__((aligned(__VA_ARGS__)))
#endif

#ifdef _DEBUG
#define GL_CHECK(stmt) do { \
        stmt; \
        lsgl_checkerror(); \
    } while (0)
#else
#define GL_CHECK(stmt) stmt
#endif

#define LSGL_BUFFER_BINDING __COUNTER__

#define lsgl_object_label(identifier, name) glObjectLabel(identifier, name, (GLsizei)strlen(#name), #name)

typedef GLfloat GLvec2[2];
typedef GLfloat GLvec3[3];
typedef GLfloat GLvec4[4];

typedef GLfloat GLmat4x4[4 * 4];
typedef GLmat4x4 GLmat4;

typedef GLint GLivec2[2];
typedef GLint GLivec3[3];
typedef GLint GLivec4[4];

typedef GLuint GLuvec2[2];
typedef GLuint GLuvec3[3];
typedef GLuint GLuvec4[4];

typedef GLbyte GLbvec2[2];
typedef GLbyte GLbvec3[3];
typedef GLbyte GLbvec4[4];

typedef GLubyte GLubvec2[2];
typedef GLubyte GLubvec3[3];
typedef GLubyte GLubvec4[4];

typedef GLushort GLusvec2[2];

typedef GLfloat GLvec2[2];
typedef GLfloat GLvec3[3];
typedef GLfloat GLvec4[4];

typedef GLfloat GLmat4x4[4 * 4];
typedef GLmat4x4 GLmat4;

typedef GLint GLivec2[2];
typedef GLint GLivec3[3];
typedef GLint GLivec4[4];

typedef GLuint GLuvec2[2];
typedef GLuint GLuvec3[3];
typedef GLuint GLuvec4[4];

typedef GLubyte GLubvec4[4];

typedef GLushort GLusvec2[2];

void lsgl_compileshader(GLuint shader, const char *src, int length);
void lsgl_linkprogram(GLuint program);
void lsgl_checkerror(void);
GLint lsgl_checkprogram(GLuint program);

#ifdef __cplusplus
}
#endif

#endif