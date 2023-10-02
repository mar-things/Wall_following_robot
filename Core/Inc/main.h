/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define NRF_TX_Pin GPIO_PIN_2
#define NRF_TX_GPIO_Port GPIOA
#define NRF_RX_Pin GPIO_PIN_3
#define NRF_RX_GPIO_Port GPIOA
#define SERVO_Pin GPIO_PIN_4
#define SERVO_GPIO_Port GPIOA
#define MOTOR_EN_Pin GPIO_PIN_6
#define MOTOR_EN_GPIO_Port GPIOA
#define MOTOR_FORWARD_Pin GPIO_PIN_1
#define MOTOR_FORWARD_GPIO_Port GPIOB
#define MOTOR_BACKWARD_Pin GPIO_PIN_2
#define MOTOR_BACKWARD_GPIO_Port GPIOB
#define LASER_SCL_Pin GPIO_PIN_11
#define LASER_SCL_GPIO_Port GPIOA
#define LASER_SDA_Pin GPIO_PIN_12
#define LASER_SDA_GPIO_Port GPIOA
#define EN_LASER_1_Pin GPIO_PIN_15
#define EN_LASER_1_GPIO_Port GPIOA
#define EN_LASER_2_Pin GPIO_PIN_0
#define EN_LASER_2_GPIO_Port GPIOD
#define EN_LASER_3_Pin GPIO_PIN_1
#define EN_LASER_3_GPIO_Port GPIOD
#define EN_LASER_4_Pin GPIO_PIN_2
#define EN_LASER_4_GPIO_Port GPIOD
#define EN_LASER_5_Pin GPIO_PIN_3
#define EN_LASER_5_GPIO_Port GPIOD
#define EN_LASER_6_Pin GPIO_PIN_3
#define EN_LASER_6_GPIO_Port GPIOB
#define EN_LASER_7_Pin GPIO_PIN_4
#define EN_LASER_7_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
