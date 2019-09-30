#include <gym.h>

typedef struct env env_t;

__declspec(dllexport) int env_observation_length(void);
__declspec(dllexport) int env_action_length(void);

__declspec(dllexport) env_t *env_create(void);
__declspec(dllexport) void env_delete(env_t *);
__declspec(dllexport) gym_t *env_gym(env_t *);
__declspec(dllexport) void env_render(env_t *);
__declspec(dllexport) void env_reset(env_t *, float *observation);
__declspec(dllexport) void env_step(env_t *, float *action, float *observation, float *reward, int *done);


