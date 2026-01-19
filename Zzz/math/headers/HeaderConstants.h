
#pragma once

export namespace zzz::math
{
	export constexpr float PI = 3.14159265358979323846f;
	export constexpr float PI_2 = PI * 2;
	export constexpr inline float DegreesToRadians(float degrees) { return degrees * (PI / 180.0f); }
	export constexpr inline float RadiansToDegrees(float radians) { return radians * (180.0f / PI); }
}