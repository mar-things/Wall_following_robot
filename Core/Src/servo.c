#include "servo.h"
#include "utils.h"

extern TIM_HandleTypeDef htim14;

uint32_t lmax;
uint32_t rmax;
uint32_t mid;
uint8_t invert;

HAL_StatusTypeDef servo_init(const uint32_t left_max, const uint32_t middle, const uint32_t right_max, uint8_t inverted)
{
	mid = middle;
	lmax = left_max;
	rmax = right_max;
	invert = inverted;

	htim14.Instance->CCR1 = mid;
	return HAL_TIM_PWM_Start(&htim14, TIM_CHANNEL_1);
}

void servo_test()
{
	servo_set(-100);
	HAL_Delay(1000);

	servo_set(0);
	HAL_Delay(1000);

	servo_set(100);
	HAL_Delay(1000);
}

HAL_StatusTypeDef servo_deinit()
{
	return HAL_TIM_PWM_Stop(&htim14, TIM_CHANNEL_1);
}

void servo_set(int8_t position)
{
    if (position >= 0)
    {
        if (invert == 1)
            htim14.Instance->CCR1 = (uint32_t)mapf(-100.0f, .0f, rmax, mid, (float)position);
        else
            htim14.Instance->CCR1 = (uint32_t)mapf(.0f, 100.f, mid, rmax, (float)position);
    }
    else
    {
        if (invert == 1)
            htim14.Instance->CCR1 = (uint32_t)mapf(.0f, 100.f, mid, lmax, (float)position);
        else
            htim14.Instance->CCR1 = (uint32_t)mapf(-100.0f, .0f, lmax, mid, (float)position);
    }
}
