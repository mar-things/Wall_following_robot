#ifndef INC_UTILS_H_
#define INC_UTILS_H_

#include "stdint.h"

//Convers degrees to radians
float deg2rad(float degrees);

//Returns 1 if value and target differ by epsilon or less, otherwise returns 0
uint8_t approx_eq(float value, float target, float epsilon);

//Linearly interpolates between min and max based on value
float lerpf(const float min, const float max, const float value);

//Maps value a from range [a_min, a_max] to range [b_min, b_max]
float mapf(const float a_min, const float a_max, const float b_min, const float b_max, const float a);

//Clamps value between two boundaries
float clampf(float value, float min, float max);

#endif
