#include <draw_locomotion.h>

void draw_locomotion_create(draw_locomotion_t *draw_locomotion) {
	draw_locomotion->image = sg_make_image(&(sg_image_desc) {
		.width = DRAW_LOCOMOTION_SIZE,
		.height = DRAW_LOCOMOTION_SIZE,
		.usage = SG_USAGE_STREAM,
		.pixel_format = SG_PIXELFORMAT_R8
	});
}
void draw_locomotion_delete(draw_locomotion_t *draw_locomotion) {

}

void draw_locomotion_frame(draw_locomotion_t *draw_locomotion, const zrc_t *zrc, const camera_t *camera, const control_t *control, float dt) {
	draw_locomotion->accumulator += dt;
	if (draw_locomotion->accumulator < DRAW_LOCOMOTION_RATE) {
		return;
	}
	draw_locomotion->accumulator = 0;
	id_t id = control->unit;
	const locomotion_t *locomotion = ZRC_GET(zrc, locomotion, id);
	if (!locomotion) {
		return;
	}
	float viewport[4] = { 0, 0, DRAW_LOCOMOTION_SIZE, DRAW_LOCOMOTION_SIZE };
	double minp = FLT_MAX;
	double maxp = -FLT_MAX;
	for (int x = 0; x < DRAW_LOCOMOTION_SIZE; ++x) {
		for (int y = 0; y < DRAW_LOCOMOTION_SIZE; ++y) {
			hmm_vec3 ro = hmm_unproject(HMM_Vec3((float)x, (float)y, 0), camera->view_projection, viewport);
			hmm_vec3 end = hmm_unproject(HMM_Vec3((float)x, (float)y, 1), camera->view_projection, viewport);
			hmm_vec3 rd = HMM_NormalizeVec3(HMM_SubtractVec3(ro, end));
			float t = isect_plane(ro, rd, HMM_Vec4(0, 0, 1, 0));
			hmm_vec3 hit = HMM_AddVec3(ro, HMM_MultiplyVec3f(rd, t));

			double potential = 0;
			for (int i = 0; i < locomotion->num_behaviors; ++i) {
				const locomotion_behavior_t *behavior = &locomotion->behaviors[i];
				double p = (*behavior)(zrc, id, cpv(hit.X, hit.Y));
				potential += p;
			}
			draw_locomotion->potential[x][y] = (float)potential;
			minp = min(minp, potential);
			maxp = max(maxp, potential);
		}
	}

	if (minp == maxp) {
		return;
	}

	for (int x = 0; x < DRAW_LOCOMOTION_SIZE; ++x) {
		for (int y = 0; y < DRAW_LOCOMOTION_SIZE; ++y) {
			double norm = (draw_locomotion->potential[x][y] - minp) / (maxp - minp);
			draw_locomotion->scaled_potential[x][y] = (uint8_t)(norm * 255);
		}
	}

	sg_update_image(draw_locomotion->image, &(sg_image_content) {
		.subimage[0][0] = {
			.size = sizeof(draw_locomotion->scaled_potential),
			.ptr = draw_locomotion->scaled_potential
		}
	});
}