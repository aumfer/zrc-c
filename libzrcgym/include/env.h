#include <gym.h>

typedef struct env env_t;

__declspec(dllexport) env_t *env_create(void);
__declspec(dllexport) void env_delete(env_t *);

__declspec(dllexport) int __cdecl env_observation_length(void);
__declspec(dllexport) int __cdecl env_action_length(void);

__declspec(dllexport) void __cdecl env_reset(env_t *, float *observation);
__declspec(dllexport) void __cdecl env_step(env_t *, float *action, float *observation, float *reward, int *done);