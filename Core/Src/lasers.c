#include "i2c.h"
#include "VL53L0X.h"
#include "lasers.h"

extern I2C_HandleTypeDef hi2c2;
laser_handle lasers[7];

void lasers_offset(uint16_t* lasers, const int8_t* offsets)
{
	for (uint8_t i = 0; i < 7; ++i)
	{
		if (lasers[i] < -offsets[i])
		{
			lasers[i] = 0;
			continue;
		}

		lasers[i] += offsets[i];
	}
}

void lasers_deinit()
{
	HAL_GPIO_WritePin(EN_LASER_1_GPIO_Port, EN_LASER_1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(EN_LASER_2_GPIO_Port, EN_LASER_2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(EN_LASER_3_GPIO_Port, EN_LASER_3_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(EN_LASER_4_GPIO_Port, EN_LASER_4_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(EN_LASER_5_GPIO_Port, EN_LASER_5_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(EN_LASER_6_GPIO_Port, EN_LASER_6_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(EN_LASER_7_GPIO_Port, EN_LASER_7_Pin, GPIO_PIN_RESET);
	HAL_Delay(10);
}

uint8_t lasers_init()
{
	//Set default parameters
	for (uint8_t i = 0; i < 7; ++i)
	{
		lasers[i].address = 0x52;
		lasers[i].timeout = 300;
		lasers[i].hi2c = &hi2c2;
	}

	//================================ Laser 1 ================================
	//Set enable pin high
	HAL_GPIO_WritePin(EN_LASER_1_GPIO_Port, EN_LASER_1_Pin, GPIO_PIN_SET);
	HAL_Delay(10);

	//Set new address
	if (laser_set_address(lasers, 0x20) != HAL_OK)
		return 0;

	//Init laser
	if (laser_init(lasers) != HAL_OK)
		return 0;

	//Start continuous ranging
	if (laser_start_continuous(lasers) != HAL_OK)
		return 0;

	//================================ Laser 2 ================================
	//Set enable pin high
	HAL_GPIO_WritePin(EN_LASER_2_GPIO_Port, EN_LASER_2_Pin, GPIO_PIN_SET);
	HAL_Delay(10);

	//Set new address
	if (laser_set_address(lasers + 1, 0x22) != HAL_OK)
		return 0;

	//Init laser
	if (laser_init(lasers + 1) != HAL_OK)
		return 0;

	//Start continuous ranging
	if (laser_start_continuous(lasers + 1) != HAL_OK)
		return 0;

	//================================ Laser 3 ================================
	//Set enable pin high
	HAL_GPIO_WritePin(EN_LASER_3_GPIO_Port, EN_LASER_3_Pin, GPIO_PIN_SET);
	HAL_Delay(10);

	//Set new address
	if (laser_set_address(lasers + 2, 0x24) != HAL_OK)
		return 0;

	//Init laser
	if (laser_init(lasers + 2) != HAL_OK)
		return 0;

	//Start continuous ranging
	if (laser_start_continuous(lasers + 2) != HAL_OK)
		return 0;

	//================================ Laser 4 ================================
	//Set enable pin high
	HAL_GPIO_WritePin(EN_LASER_4_GPIO_Port, EN_LASER_4_Pin, GPIO_PIN_SET);
	HAL_Delay(10);

	//Set new address
	if (laser_set_address(lasers + 3, 0x26) != HAL_OK)
		return 0;

	//Init laser
	if (laser_init(lasers + 3) != HAL_OK)
		return 0;

	//Start continuous ranging
	if (laser_start_continuous(lasers + 3) != HAL_OK)
		return 0;

	//================================ Laser 5 ================================
	//Set enable pin high
	HAL_GPIO_WritePin(EN_LASER_5_GPIO_Port, EN_LASER_5_Pin, GPIO_PIN_SET);
	HAL_Delay(10);

	//Set new address
	if (laser_set_address(lasers + 4, 0x28) != HAL_OK)
		return 0;

	//Init laser
	if (laser_init(lasers + 4) != HAL_OK)
		return 0;

	//Start continuous ranging
	if (laser_start_continuous(lasers + 4) != HAL_OK)
		return 0;

	//================================ Laser 6 ================================
	//Set enable pin high
	HAL_GPIO_WritePin(EN_LASER_6_GPIO_Port, EN_LASER_6_Pin, GPIO_PIN_SET);
	HAL_Delay(10);

	//Set new address
	if (laser_set_address(lasers + 5, 0x30) != HAL_OK)
		return 0;

	//Init laser
	if (laser_init(lasers + 5) != HAL_OK)
		return 0;

	//Start continuous ranging
	if (laser_start_continuous(lasers + 5) != HAL_OK)
		return 0;

	//================================ Laser 7 ================================
	//Set enable pin high
	HAL_GPIO_WritePin(EN_LASER_7_GPIO_Port, EN_LASER_7_Pin, GPIO_PIN_SET);
	HAL_Delay(10);

	//Set new address
	if (laser_set_address(lasers + 6, 0x32) != HAL_OK)
		return 0;

	//Init laser
	if (laser_init(lasers + 6) != HAL_OK)
		return 0;

	//Start continuous ranging
	if (laser_start_continuous(lasers + 6) != HAL_OK)
		return 0;

	return 1;
}

HAL_StatusTypeDef lasers_read(uint16_t* lasers_mm)
{
	HAL_StatusTypeDef status;
	for (uint8_t i = 0; i < 7; ++i)
	{
		status = laser_read_continuous(lasers + i, lasers_mm + i);
		if (status != HAL_OK)
			return status;
	}
	return HAL_OK;
}
