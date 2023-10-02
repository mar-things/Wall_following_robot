#ifndef INC_SERVO_H_
#define INC_SERVO_H_

#include "tim.h"

//Start servo PWM
//left_max and right_max are parameters that specify the maximum wheel rotation.
//Middle position is 750.
HAL_StatusTypeDef servo_init(const uint32_t left_max, const uint32_t middle, const uint32_t right_max, uint8_t inverted);

HAL_StatusTypeDef servo_deinit();

void servo_test();

//Adjust PWM based on position.
//Position can be between -100 and +100.
//-100 means full left, +100 means full right
void servo_set(int8_t position);

#endif
