
#include "pch.h"

export module Math;

export import Vector3;
export import Vector4;
export import Matrix4x4;

export namespace zzz::math
{
	export constexpr float Pi = 3.14159265358979323846f;
	export constexpr inline float DegreesToRadians(float degrees) { return degrees * (Pi / 180.0f); }
	export constexpr inline float RadiansToDegrees(float radians) { return radians * (180.0f / Pi); }
}