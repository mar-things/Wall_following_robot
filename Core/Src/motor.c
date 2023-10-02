#include "tim.h"
#include "motor.h"

extern TIM_HandleTypeDef htim3;
uint8_t hbridge_type;
uint8_t motor_inverted;

int8_t last_speed = 0;
uint32_t boost_time = 0;

HAL_StatusTypeDef motor_init(uint8_t hbridge, uint8_t inverted)
{
	hbridge_type = hbridge;
	motor_inverted = inverted;
	htim3.Instance->CCR1 = 0;
	htim3.Instance->CCR4 = 0;

	const HAL_StatusTypeDef status = HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

	if (status != HAL_OK)
		return status;

	return HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
}

HAL_StatusTypeDef motor_deinit()
{
	const HAL_StatusTypeDef status = HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);

	if (status != HAL_OK)
		return status;

	return HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_4);
}

void motor_set_speed(int8_t speed, int8_t boost)
{
	if (boost_time != 0)
	{
		if (HAL_GetTick() - boost_time >= 300)
			boost_time = 0;
		else
			return;
	}

	if (boost_time == 0 && boost != 0 && speed != last_speed && last_speed == 0)
	{
		boost_time = HAL_GetTick();

		if (speed >= 0)
			speed = boost;
		else
			speed = -boost;
	}

	if (motor_inverted == 1)
		speed *= -1;

	if (hbridge_type == 0)
	{
		if (speed == 0) //Neutral
		{
			HAL_GPIO_WritePin(MOTOR_BACKWARD_GPIO_Port, MOTOR_BACKWARD_Pin, GPIO_PIN_RESET);
			htim3.Instance->CCR4 = 0;
			htim3.Instance->CCR1 = 0;
		}
		else if (speed > 0) //Forward
		{
			HAL_GPIO_WritePin(MOTOR_BACKWARD_GPIO_Port, MOTOR_BACKWARD_Pin, GPIO_PIN_RESET);
			htim3.Instance->CCR4 = 100;
			htim3.Instance->CCR1 = (uint32_t)(speed);
		}
		else //Backward
		{
			HAL_GPIO_WritePin(MOTOR_BACKWARD_GPIO_Port, MOTOR_BACKWARD_Pin, GPIO_PIN_SET);
			htim3.Instance->CCR4 = 0;
			htim3.Instance->CCR1 = (uint32_t)(-speed);
		}
	}
	else
	{
		if (speed == 0) //Neutral
		{
			htim3.Instance->CCR1 = 0;
			htim3.Instance->CCR4 = 0;
		}
		else if (speed > 0) //Forward
		{
			htim3.Instance->CCR4 = 0;
			htim3.Instance->CCR1 = (uint32_t)(speed);
		}
		else //Backward
		{
			htim3.Instance->CCR4 = (uint32_t)(-speed);
			htim3.Instance->CCR1 = 0;
		}
	}

	last_speed = speed;
}
