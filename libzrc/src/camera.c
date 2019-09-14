#include <camera.h>
#include <sokol_app.h>

void camera_update(camera_t *camera) {
	float w = (float)sapp_width();
	float h = (float)sapp_height();

	//camera->view = HMM_Mat4d(1);
	//camera->projection = HMM_Orthographic(0.0f, w, 0.0f, h, 0.0f, 1.0f);

	camera->view = HMM_LookAt(HMM_Vec3(camera->position[0], camera->position[1], camera->zoom), HMM_Vec3(camera->target[0], camera->target[1], 0), HMM_Vec3(0, 0, 1));
	camera->projection = HMM_Perspective(103.0f, w / h, 0.01f, 10000.0f);

	camera->view_projection = HMM_MultiplyMat4(camera->projection, camera->view);
	camera->inv_view_projection = hmm_inverse(camera->view_projection);
}