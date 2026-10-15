#pragma once
#include <iostream>

constexpr float PI = 3.141592653589793f;

inline bool is_nearly_zero(float Value, float Epsilon = 1e-6f)
{
	return std::fabs(Value) < Epsilon;
}
inline float degrees_to_radians(float degrees)
{
	return degrees * (PI / 180.0f);
};

inline float radians_to_degrees(float radians)
{
	return radians * (180.0f / PI);
};
inline constexpr float lerp(float a, float b, float t) {
	return a + t * (b - a);
}

struct Vec3 {
	// vec4 constructor
	union {
		struct { float x, y, z; };
		struct { float r, g, b; };
		float data[3];
	};

	float length() {
		return std::hypot(x, y, z);
	}
	float length_squared() {
		return x * x + y * y + z * z;
	}
};

struct Vec4 {
	// vec3 constructor
	union {
		struct { float x, y, z,w; };
		struct { float r, g, b,a; };
		float data[4];
	};
};

struct Vec2 {

};


float distance(Vec3 a, Vec3 b) {
	return std::hypot(a.x - b.x, a.y - b.y, a.z - b.z);
}

struct Mat4 {

};

