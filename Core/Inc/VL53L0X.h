#ifndef INC_VL53L0X_H_
#define INC_VL53L0X_H_

#include "stm32g0xx_hal.h"

typedef struct
{
	//Setup variables
	I2C_HandleTypeDef* hi2c;
	uint16_t timeout;
	uint8_t address;

	// Internal state variables
	uint8_t g_stop;
} laser_handle;

HAL_StatusTypeDef laser_init(laser_handle* laser);
HAL_StatusTypeDef laser_set_address(laser_handle* laser, uint8_t new_address);
HAL_StatusTypeDef laser_start_continuous(laser_handle* laser);
HAL_StatusTypeDef laser_read_continuous(laser_handle* laser, uint16_t* mm);

#endif
