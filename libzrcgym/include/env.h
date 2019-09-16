#include <gym.h>

__declspec(dllexport) int env_observation_length(void);
__declspec(dllexport) int env_action_length(void);

__declspec(dllexport) void env_reset(void);
__declspec(dllexport) void env_step(float *action, float *observation, float *reward, int *done);