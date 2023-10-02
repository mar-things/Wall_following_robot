/* USER CODE BEGIN Header */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "utils.h"
#include "servo.h"
#include "motor.h"
#include "lasers.h"
#include "nrf.h"
#include "wall_follow.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint16_t laser_values[7];
uint8_t initialized = 0;

//Parameters sent from app
int8_t laser_offsets[7];

//Controls
uint8_t auto_mode = 0;
uint8_t followed_wall = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void handle_communications();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  memset(laser_offsets, 0, sizeof(laser_offsets));
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  //NRF's UART interface produces noise on initialization. STM picks up this noise and offsets the UART DMA RX buffer.
  //To avoid this wait for NRF to finish it's initialization and then initialize STM.
  HAL_Delay(2000);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_TIM14_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  if (lasers_init() == 0) return -1;
  if (nrf_init(1000) != HAL_OK) return -1;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  //Read/write data from/to NRF
	  handle_communications();

	  if (initialized != 1)
		  continue;

	  //If at least one laser fails to return a measurement restart them all
	  if (lasers_read(laser_values) != HAL_OK)
	  {
		  lasers_deinit();

		  if (lasers_init() == 0)
			  return -1;

		  continue;
	  }

	  //Apply laser offsets
	  lasers_offset(laser_values, laser_offsets);

	  //Check for timeout
	  if (nrf_timeout() == 1)
	  {
		  initialized = 0;
		  servo_deinit();
		  motor_deinit();
		  continue;
	  }

	  //Follow wall if auto_mode is 1
	  if (auto_mode == 1)
		  auto_controls(laser_values, followed_wall, 0);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void handle_communications()
{
	if (nrf_available() != 1)
		return;

	nrf_update_heartbeat();

	const uint8_t* buffer = nrf_rx_buffer();
	const uint8_t id = buffer[0];

	if (id == 1) //app sent initialization parameters
	{
		if (initialized == 1)
		{
			servo_deinit();
			motor_deinit();
			initialized = 0;
		}

		for (uint8_t i = 0; i < 7; ++i)
			laser_offsets[i] = (int8_t)buffer[i + 1];

		const uint8_t invert_servo = *((uint8_t*)(buffer + 14));
		uint16_t servo_max_left, servo_middle, servo_max_right;

		memcpy(&servo_max_left, buffer + 8, 2);
		memcpy(&servo_middle, buffer + 10, 2);
		memcpy(&servo_max_right, buffer + 12, 2);

		//Calculate timer's CCR values from angles
		servo_max_left = (uint16_t)((float)servo_max_left / 180.f * 1141.f) + 174;
		servo_middle = (uint16_t)((float)servo_middle / 180.f * 1141.f) + 174;
		servo_max_right = (uint16_t)((float)servo_max_right / 180.f * 1141.f) + 174;

		servo_init(servo_max_left, servo_middle, servo_max_right, invert_servo);

		nrf_write(id, 0, 0); //send ack
		return;
	}
	else if (id == 2)
	{
		const uint8_t invert_motor = *((uint8_t*)(buffer + 1));
		const uint8_t hbridge = *((uint8_t*)(buffer + 2));

		motor_init(hbridge, invert_motor);

		uint16_t min_laser, max_laser, target, radius, servo_sensitivity;
		uint8_t laser_amount, speed;

		memcpy(&min_laser, buffer + 3, 2);
		memcpy(&max_laser, buffer + 5, 2);
		memcpy(&radius, buffer + 7, 2);
		memcpy(&target, buffer + 9, 2);

		speed = *((uint8_t*)(buffer + 11));
		laser_amount = *((uint8_t*)(buffer + 12));

		memcpy(&servo_sensitivity, buffer + 13, 2);

		auto_controls_init(
				speed,
				max_laser,
				min_laser,
				target,
				radius,
				laser_amount,
				(float)servo_sensitivity / 100.f);

		initialized = 1;

		nrf_write(id, 0, 0); //send ack
		return;
	}
	else if (id == 4) //app sent controls
	{
		auto_mode = *((uint8_t*)(buffer + 1));
		followed_wall = *((uint8_t*)(buffer + 2));

		if (auto_mode == 0)
		{
			servo_set(*((int8_t*)(buffer + 3)));
			motor_set_speed(*((int8_t*)(buffer + 4)), 0);
		}

		nrf_write(id, (const uint8_t*)laser_values, sizeof(laser_values)); //send ack
		return;
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
