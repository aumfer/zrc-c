#pragma once

#include <HandmadeMath.h>

#define snorm(v) ((v)*2-1)
#define unorm(v) (((v)+1)/2)

#define clamp(v, minv, maxv) (min(maxv, max(minv, v)))

hmm_mat4 hmm_inverse(hmm_mat4);
hmm_vec3 hmm_unproject(hmm_vec3 win, hmm_mat4 proj, float viewport[4]);

float isect_plane(hmm_vec3 ro, hmm_vec3 rd, hmm_vec4 p);

// blas
float sdot(const int *n, const float *sx, const int *incx, const float *sy, const int *incy);