#include <gym.h>

typedef struct env env_t;

__declspec(dllexport) env_t *env_create(void);
__declspec(dllexport) void env_delete(env_t *);
__declspec(dllexport) gym_t *env_gym(env_t *);
__declspec(dllexport) void env_render(env_t *);
__declspec(dllexport) void env_reset(env_t *);

__declspec(dllexport) int env_locomotion_obs_length(void);
__declspec(dllexport) int env_locomotion_act_length(void);

__declspec(dllexport) int env_sense_obs_length(void);
__declspec(dllexport) int env_sense_act_length(void);

__declspec(dllexport) void env_reset_locomotion(env_t *, float *observation);
__declspec(dllexport) void env_step_locomotion(env_t *, float *action, float *observation, float *reward, int *done);

__declspec(dllexport) void env_reset_sense(env_t *, float *observation);
__declspec(dllexport) void env_step_sense(env_t *, float *action, float *observation, float *reward, int *done);