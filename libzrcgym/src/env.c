#include <env.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <zrc_draw.h>
#include <tinycthread.h>

#define SOKOL_IMPL
#define SOKOL_NO_ENTRY
//#define SOKOL_D3D11
#define SOKOL_GLCORE33
#include <sokol_app.h>
#include <sokol_gfx.h>

typedef struct env {
	unsigned num_resets;
	gym_t *gym;

	zrc_draw_t *zrc_draw;
	thrd_t draw_thrd;
	mtx_t draw_mtx;
} env_t;

static int has_stm;

env_t *env_create(void) {
	printf("env_create %zu\n", sizeof(env_t));
	if (!has_stm) {
		stm_setup();
		has_stm = 1;
	}
	env_t *env = calloc(1, sizeof(env_t));
	mtx_init(&env->draw_mtx, mtx_plain);
	return env;
}
void env_delete(env_t *env) {
	puts("env_delete");
	if (env->gym) {
		gym_delete(env->gym);
	}
	mtx_destroy(&env->draw_mtx);
	free(env);
}

int env_locomotion_obs_length(void) {
	return AI_LOCOMOTION_OBS_LENGTH;
}
int env_locomotion_act_length(void) {
	return AI_LOCOMOTION_ACT_LENGTH;
}
int env_sense_obs_length(void) {
	return AI_SENSE_OBS_LENGTH;
}
int env_sense_act_length(void) {
	return AI_SENSE_ACT_LENGTH;
}

void env_reset(env_t *env) {
	mtx_lock(&env->draw_mtx);

	++env->num_resets;
	printf("env_reset %u\n", env->num_resets);
	srand((unsigned)(stm_now() & UINT32_MAX));
	if (env->gym) {
		gym_delete(env->gym);
		free(env->gym);
		env->gym = 0;
	}
	env->gym = calloc(1, sizeof(gym_t));
	if (!env->gym) {
		fputs("oom", stderr);
	}
	gym_create(env->gym);

	mtx_unlock(&env->draw_mtx);

	// hack
	for (int i = 0; i < 3; ++i) {
		gym_update(env->gym);
	}
}

void env_reset_locomotion(env_t *env, float *observation) {
	env_reset(env);

	zrc_t *zrc = &env->gym->zrc;
	id_t agent = env->gym->agent;

	ai_t *ai = ZRC_GET_WRITE(zrc, ai, agent);
	if (ai) {
		ai->train_flags |= AI_TRAIN_LOCOMOTION;
	}

	ai_observe_locomotion_train(zrc, agent, observation);
}

void env_reset_sense(env_t *env, float *observation) {
	env_reset(env);

	zrc_t *zrc = &env->gym->zrc;
	id_t agent = env->gym->agent;

	ai_t *ai = ZRC_GET_WRITE(zrc, ai, agent);
	if (ai) {
		ai->train_flags |= AI_TRAIN_SENSE;
	}

	ai_observe_sense(zrc, agent, observation);
}

void env_step_locomotion(env_t *env, float *action, float *observation, float *reward, int *done) {
	if (!env->gym) return;
	gym_t *gym = env->gym;
	zrc_t *zrc = &env->gym->zrc;
	id_t agent = env->gym->agent;

	ai_t *ai = ZRC_GET(zrc, ai, agent);
	
	float t = zrc->frame * TICK_RATE;

	for (int i = 0; i < 1; ++i) {
		//printf("%u act", zrc.frame);
		ai_act_locomotion(zrc, agent, action);
		//puts(" done");
		//printf("%u update", zrc.frame);
		gym_update(gym);
		//puts(" done");

		if (ai) {
			*reward += ai->reward;
			if (ai->done) {
				*done = 1;
				printf("done (won) %.2f\n", t);
			}
		} else {
			*done = 1;
			printf("done (dead) %.2f\n", t);
		}
	}

	// 1 minute
	if (t > 60) {
		*done = 1;
		printf("done (time) %2.f\n", t);
	}

	//printf("%u obs", zrc.frame);
	ai_observe_locomotion_train(zrc, agent, observation);
	//puts(" done");

	if (*done) {
		ai_t *aip = ZRC_GET_PREV(zrc, ai, agent);
		printf("reward %.2f\n", aip->total_reward);
	}
}

void env_step_sense(env_t *env, float *action, float *observation, float *reward, int *done) {
	if (!env->gym) return;
	gym_t *gym = env->gym;
	zrc_t *zrc = &env->gym->zrc;
	id_t agent = env->gym->agent;

	ai_t *ai = ZRC_GET(zrc, ai, agent);

	float t = zrc->frame * TICK_RATE;

	for (int i = 0; i < 1; ++i) {
		//printf("%u act", zrc.frame);
		ai_act_sense(zrc, agent, action);
		//puts(" done");
		//printf("%u update", zrc.frame);
		gym_update(gym);
		//puts(" done");

		if (ai) {
			*reward += ai->reward;
			if (ai->done) {
				*done = 1;
				printf("done (won) %.2f\n", t);
			}
		}
		else {
			*done = 1;
			printf("done (dead) %.2f\n", t);
		}
	}

	// 1 minute
	if (t > 60) {
		*done = 1;
		printf("done (time) %2.f\n", t);
	}

	ai_observe_sense(zrc, agent, observation);
	

	if (*done) {
		ai_t *aip = ZRC_GET_PREV(zrc, ai, agent);
		printf("reward %.2f\n", aip->total_reward);
	}
}

static void init(void *data) {
	env_t *env = data;

	zrc_draw_create(env->zrc_draw);
	env->zrc_draw->control.fixed_camera = 1;
	env->zrc_draw->draw.not_real_time = 1;
}

static void frame(void *data) {
	env_t *env = data;

	mtx_lock(&env->draw_mtx);
	if (env->gym) {
		env->zrc_draw->control.unit = env->gym->agent;
		zrc_draw_frame(env->zrc_draw, &env->gym->zrc);
	}
	mtx_unlock(&env->draw_mtx);
}

static void cleanup(void *data) {
	env_t *env = data;

	zrc_draw_delete(env->zrc_draw);
}

static void event(const sapp_event *e, void *data) {
	env_t *env = data;

	ui_event_cb(e, &env->zrc_draw->ui);
}

static int draw(void *data) {
	env_t *env = data;

	sapp_run(&(sapp_desc) {
		.user_data = env,
		.init_userdata_cb = init,
		.frame_userdata_cb = frame,
		.cleanup_userdata_cb = cleanup,
		.event_userdata_cb = event,
		.width = 1920 / 2,
		.height = 1080 / 2,
		.sample_count = 4,
		.window_title = "-= zen rat city =-",
	});

	return 0;
}

void env_render(env_t *env) {
	if (!env->zrc_draw) {
		env->zrc_draw = calloc(1, sizeof(zrc_draw_t));
		thrd_create(&env->draw_thrd, draw, env);
	}
}

gym_t *env_gym(env_t *env) {
	return env->gym;
}