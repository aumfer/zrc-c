#include <lsgl.h>
#include <stdio.h>
#include <assert.h>

void lsgl_compileshader(GLuint shader, const char *src, int length) {
	glShaderSource(shader, 1, &src, length ? &length : NULL);
	glCompileShader(shader); {
		GLint shader_log_length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &shader_log_length);
		if (shader_log_length > 1) {
			GLchar *shader_log = (GLchar *)calloc(shader_log_length, sizeof(GLchar));
			if (shader_log) {
				glGetShaderInfoLog(shader, shader_log_length, NULL, shader_log);
				puts(shader_log);
				assert(0);
				free(shader_log);
			}
		}
	}
}

void lsgl_linkprogram(GLuint program) {
	glLinkProgram(program); {
		GLint program_log_length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &program_log_length);
		if (program_log_length > 1) {
			GLchar *program_log = (GLchar *)calloc(program_log_length, sizeof(GLchar));
			if (program_log) {
				glGetProgramInfoLog(program, program_log_length, NULL, program_log);
				puts(program_log);
				free(program_log);
			}
		}

#if _DEBUG
		GLint link_status;
		glGetProgramiv(program, GL_LINK_STATUS, &link_status);
		assert(link_status == GL_TRUE);
#endif
	}
}

void lsgl_checkerror(void) {
	int error_count = 0;
	do {
		GLenum error = glGetError();
		if (error != GL_NO_ERROR) {
			++error_count;
			//puts((const char *)glewGetErrorString(error));
		}
		else {
			break;
		}
	} while (1);
	if (error_count) {
		assert(0);
	}
}

GLint lsgl_checkprogram(GLuint program) {
	GLint is_valid;
	glValidateProgram(program);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &is_valid);
	assert(is_valid == GL_TRUE);
	return is_valid;
}