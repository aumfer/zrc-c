#include <gym.h>

__declspec(dllexport) int __cdecl env_observation_length(void);
__declspec(dllexport) int __cdecl env_action_length(void);

__declspec(dllexport) void __cdecl env_reset(float *observation);
__declspec(dllexport) void __cdecl env_step(float *action, float *observation, float *reward, int *done);