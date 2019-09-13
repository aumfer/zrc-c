GLSL(
const float M_PI = 3.14159265358979323846;
const float M_PHI = 1.6180339887498948482045868343656;
const float FLT_EPSILON = 9.99999974737875E-06;
const float FLT_MAX = 3.402823466e+38F;

float unorm(float v) {
	return v * 0.5 + 0.5;
}
vec2 unorm(vec2 v) {
	return v * 0.5 + 0.5;
}
vec3 unorm(vec3 v) {
	return v * 0.5 + 0.5;
}
vec4 unorm(vec4 v) {
	return v * 0.5 + 0.5;
}
float snorm(float v) {
    return v * 2.0 - 1.0;
}
vec2 snorm(vec2 v) {
	return v * 2.0 - 1.0;
}
vec3 snorm(vec3 v) {
	return v * 2.0 - 1.0;
}
vec4 snorm(vec4 v) {
	return v * 2.0 - 1.0;
}

float square(float X) {
	return X * X;
}

float mincomp(vec2 v) { return min(v.x, v.y); }
float maxcomp(vec2 v) { return max(v.x, v.y); }
float mincomp(vec3 v) { return min(min(v.x, v.y), v.z); }
float maxcomp(vec3 v) { return max(max(v.x, v.y), v.z); }

float smin(float a, float b, float k) {
    // http://iquilezles.org/www/articles/smin/smin.htm
    float res = exp(-k * a) + exp(-k * b);
    return -log(res) / k;
}

float cubemap_layerface(float layer_orig) {
	float layer = floor(layer_orig / 6.0);
	float face = layer_orig - (layer * 6.0);
	return face;
}

void swap(inout float a, inout float b) {
	float t = a;
	a = b;
	b = t;
}
float gamma(float n) {
	return (n * FLT_EPSILON) / (1.0 - n * FLT_EPSILON);
}

mat4 perspective(float fovy, float aspect, float zNear, float zFar) {
	float tanHalfFovy = tan(fovy / 2);

	mat4 Result = mat4(0);
	Result[0][0] = 1 / (aspect * tanHalfFovy);
	Result[1][1] = 1 / (tanHalfFovy);
	Result[2][3] = - 1;
	Result[2][2] = - (zFar + zNear) / (zFar - zNear);
	Result[3][2] = - (2 * zFar * zNear) / (zFar - zNear);

	return Result;
}

mat4 ortho(float left, float right, float bottom, float top, float zNear, float zFar) {
	mat4 Result = mat4(1);
	Result[0][0] = 2 / (right - left);
	Result[1][1] = 2 / (top - bottom);
	Result[3][0] = - (right + left) / (right - left);
	Result[3][1] = - (top + bottom) / (top - bottom);
	Result[2][2] = - 2 / (zFar - zNear);
	Result[3][2] = - (zFar + zNear) / (zFar - zNear);
	return Result;
}

mat4 lookAt(vec3 eye, vec3 center, vec3 up) {
	vec3 f = normalize(center - eye);
	vec3 s = normalize(cross(f, up));
	vec3 u = cross(s, f);

	mat4 Result = mat4(1);
	Result[0][0] = s.x;
	Result[1][0] = s.y;
	Result[2][0] = s.z;
	Result[0][1] = u.x;
	Result[1][1] = u.y;
	Result[2][1] = u.z;
	Result[0][2] =-f.x;
	Result[1][2] =-f.y;
	Result[2][2] =-f.z;
	Result[3][0] =-dot(s, eye);
	Result[3][1] =-dot(u, eye);
	Result[3][2] = dot(f, eye);
	return Result;
}

mat2 rotate2d(float _angle){
    return mat2(cos(_angle),-sin(_angle),
                sin(_angle),cos(_angle));
}

vec3 rotateX(vec3 v, float angle) {
	vec3 Result = v;
	float Cos = cos(angle);
	float Sin = sin(angle);

	Result.y = v.y * Cos - v.z * Sin;
	Result.z = v.y * Sin + v.z * Cos;
	return Result;
}
vec4 rotateX(vec4 v, float angle) {
	vec4 Result = v;
	float Cos = cos(angle);
	float Sin = sin(angle);

	Result.y = v.y * Cos - v.z * Sin;
	Result.z = v.y * Sin + v.z * Cos;
	return Result;
}

vec3 rotateY(vec3 v, float angle) {
	vec3 Result = v;
	float Cos = cos(angle);
	float Sin = sin(angle);

	Result.x =  v.x * Cos + v.z * Sin;
	Result.z = -v.x * Sin + v.z * Cos;
	return Result;
}
vec4 rotateY(vec4 v, float angle) {
	vec4 Result = v;
	float Cos = cos(angle);
	float Sin = sin(angle);

	Result.x =  v.x * Cos + v.z * Sin;
	Result.z = -v.x * Sin + v.z * Cos;
	return Result;
}

vec2 rotateZ(vec2 v, float angle) {
	float Cos = cos(angle);
	float Sin = sin(angle);

	vec2 Result;
	Result.x = v.x * Cos - v.y * Sin;
	Result.y = v.x * Sin + v.y * Cos;
	return Result;
}
vec3 rotateZ(vec3 v, float angle) {
	vec3 Result = v;
	float Cos = cos(angle);
	float Sin = sin(angle);

	Result.x = v.x * Cos - v.y * Sin;
	Result.y = v.x * Sin + v.y * Cos;
	return Result;
}
vec4 rotateZ(vec4 v, float angle) {
	vec4 Result = v;
	float Cos = cos(angle);
	float Sin = sin(angle);

	Result.x = v.x * Cos - v.y * Sin;
	Result.y = v.x * Sin + v.y * Cos;
	return Result;
}

vec2 to_dir(float angle) {
	vec2 dir = vec2(
		cos(angle),
		sin(angle)
	);
	return dir;
}
float to_angle(vec2 dir) {
	float angle = atan(dir.y, dir.x);
	return angle;
}

mat4 mat4_scale(float x, float y, float z){
    return mat4(
        vec4(x,   0.0, 0.0, 0.0),
        vec4(0.0, y,   0.0, 0.0),
        vec4(0.0, 0.0, z,   0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );
}

mat4 mat4_translate(float x, float y, float z){
    return mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(x,   y,   z,   1.0)
    );
}

//==========================================================================================
// indexing
//==========================================================================================

int index2d(ivec2 p, ivec2 bounds) {
	int index = p.y +
				p.x * bounds.x;
	return index;
}
int index3d(ivec3 p, ivec3 bounds) {
	int index = p.z +
				p.y * bounds.y +
				p.x * bounds.x * bounds.y;
	return index;
}
int index4d(ivec4 p, ivec4 bounds) {
	int index = p.w +
				p.z * bounds.z +
				p.y * bounds.y * bounds.x +
				p.x * bounds.z * bounds.y * bounds.x;
	return index;
}

//==========================================================================================
// hashes
//==========================================================================================

float hash( float n ){return fract(sin(n)*43758.5453);}

float hash1(vec2 p) {
	p = 50.0*fract(p*0.3183099);
	return fract(p.x*p.y*(p.x + p.y));
}

float hash1(float n) {
	return fract(n*17.0*fract(n*0.3183099));
}

vec2 hash2(float n) { return fract(sin(vec2(n, n + 1.0))*vec2(43758.5453123, 22578.1459123)); }

vec2 hash2(vec2 p) {
	const vec2 k = vec2(0.3183099, 0.3678794);
	p = p*k + k.yx;
	return fract(16.0 * k*fract(p.x*p.y*(p.x + p.y)));
}

//==========================================================================================
// rng
//==========================================================================================

// http://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash(uint x) {
	x += (x << 10u);
	x ^= (x >> 6u);
	x += (x << 3u);
	x ^= (x >> 11u);
	x += (x << 15u);
	return x;
}
// Compound versions of the hashing algorithm I whipped together.
uint hash(uvec2 v) { return hash(v.x ^ hash(v.y)); }
uint hash(uvec3 v) { return hash(v.x ^ hash(v.y) ^ hash(v.z)); }
uint hash(uvec4 v) { return hash(v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w)); }

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct(uint m) {
	const uint ieeeMantissa = 0x007FFFFFu;	// binary32 mantissa bitmask
	const uint ieeeOne = 0x3F800000u;		// 1.0 in IEEE binary32

	m &= ieeeMantissa;						// Keep only mantissa bits (fractional part)
	m |= ieeeOne;							// Add fractional part to 1.0

	float  f = uintBitsToFloat(m);			// Range [1:2]
	return f - 1.0;							// Range [0:1]
}

uniform uvec4 random_seed;
uvec4 seed = random_seed;
void random_init1ui(uint x) {
	seed ^= uvec4(x, ~x, -x, -~x);
}
void random_init2ui(uvec2 x) {
	seed ^= uvec4(x.x, x.y, ~x.x, ~x.y);
}
void random_init2f(vec2 v) {
	uvec2 x = floatBitsToUint(v);
	seed ^= uvec4(x.x, x.y, ~x.x, ~x.y);
}
void random_init3f(vec3 v) {
	uvec3 x = floatBitsToUint(v);
	seed ^= uvec4(x.x, x.y, x.z, x.x ^ x.y ^ x.z);
}
void random_init4f(vec4 v) {
	uvec4 x = floatBitsToUint(v);
	seed ^= uvec4(x.x, x.y, x.z, x.w);
}
float random1f() {
	return floatConstruct(hash(seed++));
}
vec2 random2f() {
	return vec2(random1f(), random1f());
}
vec3 random3f() {
	return vec3(random1f(), random1f(), random1f());
}
float normal1f() {
	return sqrt(random1f()) * cos(random1f() * M_PI * 2.0);
}
vec2 normal2f() {
	return vec2(normal1f(), normal1f());
}

//==========================================================================================
// noise
//==========================================================================================

//iq's ubiquitous 3d noise
float noise(vec3 p) {
	vec3 ip = floor(p);
    vec3 f = fract(p);
	f = f*f*(3.0-2.0*f);
	
	vec2 uv = (ip.xy+vec2(37.0,17.0)*ip.z) + f.xy;
	vec2 rg = hash2(uv);
	return mix(rg.x, rg.y, f.z);
}

//==========================================================================================
// encoding
//==========================================================================================

// https://www.shadertoy.com/view/4t2XWK
float madfrac(float a, float b) { return a*b - floor(a*b); }
vec2  madfrac(vec2 a, float b) { return a*b - floor(a*b); }

float encode_normal(vec3 p, float n) {
	float phi = min(atan(p.y, p.x), M_PI), cosTheta = p.z;

	float k = max(2.0, floor(log(n * M_PI * sqrt(5.0) * (1.0 - cosTheta*cosTheta)) / log(M_PHI*M_PHI)));
	float Fk = pow(M_PHI, k) / sqrt(5.0);

	vec2 F = vec2(round(Fk), round(Fk * M_PHI));

	vec2 ka = -2.0*F / n;
	vec2 kb = 2.0*M_PI*madfrac(F + 1.0, M_PHI - 1.0) - 2.0*M_PI*(M_PHI - 1.0);
	mat2 iB = mat2(ka.y, -ka.x, -kb.y, kb.x) / (ka.y*kb.x - ka.x*kb.y);

	vec2 c = floor(iB * vec2(phi, cosTheta - (1.0 - 1.0 / n)));
	float d = 8.0;
	float j = 0.0;
	for (int s = 0; s<4; s++)
	{
		vec2 uv = vec2(float(s - 2 * (s / 2)), float(s / 2));

		float cosTheta = dot(ka, uv + c) + (1.0 - 1.0 / n);

		cosTheta = clamp(cosTheta, -1.0, 1.0)*2.0 - cosTheta;
		float i = floor(n*0.5 - cosTheta*n*0.5);
		float phi = 2.0*M_PI*madfrac(i, M_PHI - 1.0);
		cosTheta = 1.0 - (2.0*i + 1.0) / n;
		float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

		vec3 q = vec3(cos(phi)*sinTheta, sin(phi)*sinTheta, cosTheta);
		float squaredDistance = dot(q - p, q - p);
		if (squaredDistance < d)
		{
			d = squaredDistance;
			j = i;
		}
	}
	return j;
}

vec3 decode_normal(float i, float n) {
	float phi = 2.0*M_PI*madfrac(i, M_PHI);
	float zi = 1.0 - (2.0*i + 1.0) / n;
	float sinTheta = sqrt(1.0 - zi*zi);
	return vec3(cos(phi)*sinTheta, sin(phi)*sinTheta, zi);
}

//==========================================================================================
// sampling
//==========================================================================================

float radicalInverse_VdC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 hammersley2d(uint i, uint N) {
	return vec2(float(i) / float(N), radicalInverse_VdC(i));
}
vec2 hammersley2d(int i, int N) {
	return vec2(float(i) / float(N), radicalInverse_VdC(uint(i)));
}

// https://www.shadertoy.com/view/MsdGzl
vec3 cosineDirection(vec2 rnd, vec3 nor) {
	// compute basis from normal
	// see http://orbit.dtu.dk/fedora/objects/orbit:113874/datastreams/file_75b66578-222e-4c7d-abdf-f7e255100209/content
	// (link provided by nimitz)
	vec3 tc = vec3(1.0 + nor.z - nor.xy*nor.xy, -nor.x*nor.y) / (1.0 + nor.z);
	vec3 uu = vec3(tc.x, tc.z, -nor.x);
	vec3 vv = vec3(tc.z, tc.y, -nor.y);

	float u = rnd.x;
	float v = rnd.y;
	float a = M_PI * 2.0 * v;

	vec3 d = sqrt(u)*(cos(a)*uu + sin(a)*vv) + sqrt(1.0 - u)*nor;
	d = normalize(d);
	return d;
}

vec2 sampleSphericalMap(vec3 v) {
	const vec2 invAtan = vec2(0.1591, 0.3183);
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

//==========================================================================================
// intersection
//==========================================================================================

float isect_sphere(vec3 ro, vec3 rd, vec4 sph) {
	vec3 oc = ro - sph.xyz;
	float b = dot(oc, rd);
	float c = dot(oc, oc) - sph.w*sph.w;
	float h = b*b - c;
	if (h<0.0) {
		return -1.0;
	}
	h = sqrt(h);
	return -b - h;
}

float isect_plane(vec3 ro, vec3 rd, vec4 p) {
	float t = -(dot(ro, p.xyz) + p.w) / dot(rd, p.xyz);
	return t;
}

float minT(float a, float b) {
	const float zfar = -1.0;
	return (a<b)? zfar: (b<0.)? (a>0.)? a: zfar: b;
}

// https://www.shadertoy.com/view/XtGXRz
float isect_aabb(vec3 ro, vec3 rd, vec3 mins, vec3 maxs) {
	vec3 t1 = (mins - ro)/rd; //https://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm
	vec3 t2 = (maxs - ro)/rd;
	vec3 tn = min(t1, t2);
	vec3 tx = max(t1, t2);
    float d = minT(mincomp(tx),maxcomp(tn)); //minT calculates the minimum positive, if n/a then returns zfar
	return d;
}

float isect_triangle( vec3 orig
                       , vec3 dir
                       , vec3 vert0
                       , vec3 vert1
                       , vec3 vert2
                       , out float u
                       , out float v
                       ) {
    // Fast, Minimum Storage Ray-Triangle Intersection
    //
    // Tomas M�ller and Ben Trumbore. Fast, minimum storage ray-triangle intersection.
    // Journal of graphics tools, 2(1):21-28, 1997
    //
    // http://www.jcenligne.fr/download/little3d/
    //     jgt%20Fast,%20Minumum%20Storage%20Ray-Triangle%20Intersection.htm

    const float JGT_RAYTRI_EPSILON = 0.000001;
	float t;

    vec3 edge1, edge2, tvec, pvec, qvec;
    float det, inv_det;

    // Find vectors for two edges sharing vert0
    edge1 = vert1 - vert0;
    edge2 = vert2 - vert0;

    // Begin calculating determinant - also used to calculate U parameter
    pvec = cross(dir, edge2);

    // If determinant is near zero, ray lies in plane of triangle
    det = dot(edge1, pvec);

    if (det > -JGT_RAYTRI_EPSILON && det < JGT_RAYTRI_EPSILON) {
        return -1.0;
	}
    inv_det = 1.0 / det;

    // Calculate distance from vert0 to ray origin
    tvec = orig - vert0;

    // Calculate U parameter and test bounds
    u = dot(tvec, pvec) * inv_det;
    if (u < 0.0 || u > 1.0) {
        return -1.0;
	}

    // Prepare to test V parameter
    qvec = cross(tvec, edge1);

    // Calculate V parameter and test bounds
    v = dot(dir, qvec) * inv_det;
    if (v < 0.0 || u + v > 1.0) {
        return -1.0;
	}

    // Calculate t, ray intersects triangle
    t = dot(edge2, qvec) * inv_det;

    return t;
}

// https://rootllama.wordpress.com/2014/05/26/point-in-polygon-test/
float isect_line(vec2 o, vec2 d, vec2 a, vec2 b ) {
    vec2 ortho = vec2( -d.y, d.x );
    vec2 aToO = vec2( o - a );
    vec2 aToB = vec2( b - a );
 
    float denom = dot( aToB, ortho );
 
    // Here would be a good time to see if denom is zero in which case the line segment and
    // the ray are parallel.
	if (denom == 0) {
		return -1;
	}
 
    float t1 = abs( aToB.x * aToO.y - aToO.x * aToB.y ) / denom;
    float t2 = dot( aToO, ortho ) / denom;
 
    if (t2 >= 0 && t2 <= 1 && t1 >= 0) {
		return t1;
	}
	return -1;
}

//==========================================================================================
// texture
//==========================================================================================

//https://en.wikipedia.org/wiki/Bilinear_filtering
float textureFilter(usampler2DArray tex, vec2 st, int layer) {
	ivec2 size = textureSize(tex, 0).xy;
	vec2 ratio = fract(st * size - 0.5);
	vec2 opposite = 1 - ratio;
	uvec4 value = textureGather(tex, vec3(st, layer));
	return
		(value.w * opposite.x + value.z * ratio.x) * opposite.y +
		(value.x * opposite.x + value.y * ratio.x) * ratio.y;
}
float textureFilter(usampler2DArray tex, vec2 st, int layer, uvec4 count) {
	ivec2 size = textureSize(tex, 0).xy;
	vec2 ratio = fract(st * size - 0.5);
	vec2 opposite = 1 - ratio;
	uvec4 x = textureGather(tex, vec3(st, layer));
	vec4 value = x / vec4(max(uvec4(1), count) * 255);
	return
		(value.w * opposite.x + value.z * ratio.x) * opposite.y +
		(value.x * opposite.x + value.y * ratio.x) * ratio.y;
}

vec3 rgb(float r, float g, float b) {
	return vec3(r, g, b) / 255;
}

vec3 pal(in float t, in vec3 a, in vec3 b, in vec3 c, in vec3 d) {
	return a + b*cos(2 * M_PI*(c*t + d));
}

float fill(float d, float k) {
    float a = 1.0 - smoothstep(0.0, k, d);
    return a;
}
float fill(float d) {
    return fill(d, 0.0015);
}
float fill2(float d) {
	//float w = fwidth( d );
	//return 1 - smoothstep( 0.5 - w, 0.5 + w, d );
	float s = d - 0.5;
	float v = s / fwidth( s );
    return 1 - clamp( v + 0.5, 0.0, 1.0 );
}

float grid_line(vec2 p) {
    float d = mincomp(abs(fract(p) - 0.5));
	return d;
}
float grid_point(vec2 p) {
	float d = maxcomp(abs(fract(p) - 0.5));
	return d;
}

float attenuation(float dist) {
	return 1 / (dist*dist);
}

//==========================================================================================
// splines
//==========================================================================================

/**
 * Tension. Default Catmul-Rom matrix
 * has tension equal to 0.5.
 *
 * Values below 0.5 will cause sharp edges,
 * values above 0.5 will produce more curly lines.
 */
const float T = 0.7;

/**
 * Catmull-Rom Matrix
 */
const mat4 CRM = mat4(-T,        2.0 - T,  T - 2.0,         T,
                       2.0 * T,  T - 3.0,  3.0 - 2.0 * T,  -T,
                      -T,        0.0,      T,               0.0,
                       0.0,      1.0,      0.0,             0.0);
/**
 * Catmull-Rom Spline Interpolation
 */
vec2 interpolate(vec2 G1, vec2 G2, vec2 G3, vec2 G4, float t) {
    vec2 A = G1 * CRM[0][0] + G2 * CRM[0][1] + G3 * CRM[0][2] + G4 * CRM[0][3];
    vec2 B = G1 * CRM[1][0] + G2 * CRM[1][1] + G3 * CRM[1][2] + G4 * CRM[1][3];
    vec2 C = G1 * CRM[2][0] + G2 * CRM[2][1] + G3 * CRM[2][2] + G4 * CRM[2][3];
    vec2 D = G1 * CRM[3][0] + G2 * CRM[3][1] + G3 * CRM[3][2] + G4 * CRM[3][3];

    return t * (t * (t * A + B) + C) + D;
}
)
