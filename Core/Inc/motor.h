#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

//Start motor PWM
//Set hbridge to 0 if using L298N
//Set hbridge to 1 if using BTS7960
HAL_StatusTypeDef motor_init(uint8_t hbridge, uint8_t inverted);

HAL_StatusTypeDef motor_deinit();

//Adjust PWM based on speed.
//Speed can be between -100 and +100.
//-100 means full speed backward, +100 means full speed forward
void motor_set_speed(int8_t speed, int8_t boost);

#endif
