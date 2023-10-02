#include "utils.h"
#include "math.h"

float deg2rad(float degrees)
{
	return degrees * 3.14159f / 180.f;
}

uint8_t approx_eq(float value, float target, float epsilon)
{
	if (fabsf(value - target) <= epsilon)
		return 1;

	return 0;
}

float lerpf(const float min, const float max, const float value)
{
	return min + (max - min) * value;
}

float mapf(const float a_min, const float a_max, const float b_min, const float b_max, const float a)
{
	return b_min + (b_max - b_min) / (a_max - a_min) * (a - a_min);
}

float clampf(float value, float min, float max)
{
	if (value <= min)
		return min;

	if (value >= max)
		return max;

	return value;
}
