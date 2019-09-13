GLSL(
    
//==========================================================================================
// distance
// https://www.shadertoy.com/view/4dfXDn
//==========================================================================================

vec2 sdTranslate(vec2 p, vec2 t) {
	return p - t;
}

float sdSmoothMerge(float d1, float d2, float k) {
    float h = clamp(0.5 + 0.5*(d2 - d1)/k, 0.0, 1.0);
    return mix(d2, d1, h) - k * h * (1.0-h);
}

float sdMerge(float d1, float d2) {
	return min(d1, d2);
}

float sdMergeExclude(float d1, float d2) {
	return min(max(-d1, d2), max(-d2, d1));
}

float sdSubstract(float d1, float d2) {
	return max(-d1, d2);
}

float sdIntersect(float d1, float d2) {
	return max(d1, d2);
}

float sdPie(vec2 p, float angle) {
	angle = radians(angle) / 2.0;
	vec2 n = vec2(cos(angle), sin(angle));
	return abs(p).x * n.x + p.y*n.y;
}

float sdCircle(vec2 p, float radius) {
	return length(p) - radius;
}

float sdTriangle(vec2 p, float radius) {
	return max(	abs(p).x * 0.866025 + 
			   	p.y * 0.5, -p.y) 
				-radius * 0.5;
}

float sdTriangle(vec2 p, float width, float height) {
	vec2 n = normalize(vec2(height, width / 2.0));
	return max(	abs(p).x*n.x + p.y*n.y - (height*n.y), -p.y);
}

float sdSemiCircle(vec2 p, float radius, float angle, float width) {
	width /= 2.0;
	radius -= width;
	return sdSubstract(sdPie(p, angle), 
					 abs(sdCircle(p, radius)) - width);
}

float sdBox(vec2 p, vec2 size, float radius) {
	size -= vec2(radius);
	vec2 d = abs(p) - size;
  	return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - radius;
}

float sdLine(vec2 p, vec2 start, vec2 end, float width) {
	vec2 dir = start - end;
	float lngth = length(dir);
	dir /= lngth;
	vec2 proj = max(0.0, min(lngth, dot((start - p), dir))) * dir;
	return length( (start - p) - proj ) - (width / 2.0);
}

float length2(in vec2 v) { return dot(v, v); }

float sdSegmentSq(in vec2 p, in vec2 a, in vec2 b) {
	vec2 pa = p - a, ba = b - a;
	float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
	return length2(pa - ba * h);
}

// cubic
vec2 udBezierSq(vec2 pos, vec2 p0, vec2 p1, vec2 p2, in vec2 p3) {
	const int kNum = 50;
	vec2 res = vec2(1e10, 0.0);
	vec2 a = p0;
	for (int i = 1; i < kNum; i++)
	{
		float t = float(i) / float(kNum - 1);
		float s = 1.0 - t;
		vec2 b = p0 * s*s*s + p1 * 3.0*s*s*t + p2 * 3.0*s*t*t + p3 * t*t*t;
		float d = sdSegmentSq(pos, a, b);
		if (d < res.x) res = vec2(d, t);
		a = b;
	}

	return vec2(res.x, res.y);
}

// quadratic
float sdBezierSq(in vec2 pos, in vec2 A, in vec2 B, in vec2 C)
{
	vec2 a = B - A;
	vec2 b = A - 2.0*B + C;
	vec2 c = a * 2.0;
	vec2 d = A - pos;
	float kk = 1.0 / dot(b, b);
	float kx = kk * dot(a, b);
	float ky = kk * (2.0*dot(a, a) + dot(d, b)) / 3.0;
	float kz = kk * dot(d, a);
	float res = 0.0;
	float p = ky - kx * kx;
	float p3 = p * p*p;
	float q = kx * (2.0*kx*kx - 3.0*ky) + kz;
	float h = q * q + 4.0*p3;
	if (h >= 0.0)
	{
		h = sqrt(h);
		vec2 x = (vec2(h, -h) - q) / 2.0;
		vec2 uv = sign(x)*pow(abs(x), vec2(1.0 / 3.0));
		float t = uv.x + uv.y - kx;
		t = clamp(t, 0.0, 1.0);
		vec2 qos = d + (c + b * t)*t;
		res = dot(qos, qos);
	}
	else
	{
		float z = sqrt(-p);
		float v = acos(q / (p*z*2.0)) / 3.0;
		float m = cos(v);
		float n = sin(v)*1.732050808;
		vec3 t = vec3(m + m, -n - m, n - m) * z - kx;
		t = clamp(t, 0.0, 1.0);
		vec2 qos = d + (c + b * t.x)*t.x;
		res = dot(qos, qos);
		qos = d + (c + b * t.y)*t.y;
		res = min(res, dot(qos, qos));
		qos = d + (c + b * t.z)*t.z;
		res = min(res, dot(qos, qos));
	}
	return res;
}

vec2 hex_corner(vec2 center, float size, int i);
float sdHex(vec2 p, vec2 center, float size, float width) {
	float d = FLT_MAX;
	vec2 start = hex_corner(center, size, 0);
	for (int i = 1; i <= 6; ++i) {
		vec2 end = hex_corner(center, size, i % 6);
		d = min(d, sdLine(p, start, end, width));
		start = end;
	}
	return d;
}

//==========================================================================================
// hexagon
//==========================================================================================

vec2 hex_corner(vec2 center, float size, int i) {
	float angle_deg = 60 * i + 30;
	float angle_rad = radians(angle_deg);
	return vec2(
		center.x + size * cos(angle_rad),
		center.y + size * sin(angle_rad)
	);
}
vec2 hex_corner(vec2 center, int i) {
	return hex_corner(center, 1.0, i);
}

vec4 hex_edge(vec2 center, float size, int i) {
	return vec4(
		hex_corner(center, size, i),
		hex_corner(center, size, (i+1)%6)
	);
}
vec4 hex_edge(vec2 center, int i) {
	return hex_edge(center, 1.0, i);
}

vec3 hex_to_cube(vec2 hex) {
	return vec3(hex.x, hex.y, -hex.x - hex.y);
}
ivec3 hex_round(vec3 cube) {
	vec3 r = round(cube);
	vec3 diff = abs(r - cube);
	if (diff.x > diff.y && diff.x > diff.z) {
		r.x = -r.y - r.z;
	} else if (diff.y > diff.z) {
		r.y = -r.x - r.z;
	} else {
		r.z = -r.x - r.y;
	}
	return ivec3(r);
}
vec2 hex_round(vec2 hex) {
	vec3 cube = hex_to_cube(hex);
	ivec3 r = hex_round(cube);
	return vec2(r.xy);
}
vec2 pixel_to_hex(vec2 pixel) {
	vec2 hex = vec2(
		pixel.x * sqrt(3)/3 - pixel.y / 3,
		pixel.y * (2.0/3.0)
	);
	return hex;
}
vec2 hex_to_pixel(vec2 hex) {
	vec2 pixel = vec2(
		sqrt(3) * (hex.x + hex.y / 2),
		(3.0/2.0) * hex.y
	);
	return pixel;
}
vec2 hex_center(vec2 pixel, float size) {
	vec2 h = pixel_to_hex(pixel) / size;
	vec3 c = hex_to_cube(h);
	vec3 rc = hex_round(c);
	vec2 rh = rc.xy;
	vec2 hp = hex_to_pixel(rh) * size;
	return hp;
}
vec2 hex_center(vec2 pixel) {
	return hex_center(pixel, 1.0);
}

// { 2d cell id, distance to border, distance to center )
vec4 hexagon(vec2 p, float size) {
	vec2 h = pixel_to_hex(p) / size;
	vec3 c = hex_to_cube(h);
	vec3 rc = hex_round(c);
	vec2 rh = rc.xy;
	vec2 hp = hex_to_pixel(rh) * size;

	float dc = length(c - rc);
	float de = sdHex(p, hp, size, 0);
	
	return vec4(rh, de, dc);
}
vec4 hexagon(vec2 p) {
	return hexagon(p, 1);
}

const ivec2 HEX_DIRECTIONS[] = {
	ivec2(+1, 0), ivec2(+1, -1), ivec2(0, -1),
	ivec2(-1, 0), ivec2(-1, +1), ivec2(0, +1)
};
ivec2 hex_neighbor(ivec2 hex, int i) {
	ivec2 neighbor = hex + HEX_DIRECTIONS[i];
	return neighbor;
}
int hex_direction_reverse(int i) {
	return (i + 3) % 6;
}
)