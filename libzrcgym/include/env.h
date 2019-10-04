#include <gym.h>

typedef struct env env_t;

__declspec(dllexport) int env_observation_length(void);
__declspec(dllexport) int env_action_length(void);

__declspec(dllexport) env_t *env_create(void);
__declspec(dllexport) void env_delete(env_t *);
__declspec(dllexport) gym_t *env_gym(env_t *);
__declspec(dllexport) void env_render(env_t *);
__declspec(dllexport) void env_reset(env_t *, rl_obs_t *observation);
__declspec(dllexport) void env_step(env_t *, rl_act_t *action, rl_obs_t *observation, float *reward, int *done);


