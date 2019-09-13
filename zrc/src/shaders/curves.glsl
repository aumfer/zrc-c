GLSL(
const int INSTANCE_POINTS = 8;
const int INSTANCE_FLAGS_NONE = 0;
const int INSTANCE_FLAGS_SELECTED = 1;
struct instance_t {
	vec2 position;
	float radius;
	uint color;
	mat4 transform;
	vec2 velocity;
	float angular_velocity;
	uint flags;
	vec2 points[INSTANCE_POINTS];
};

layout(std430, binding = 0) buffer instance_buffer
{
	instance_t instances[];
};
)