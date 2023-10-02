#include "string.h"
#include "nrf.h"

extern UART_HandleTypeDef huart2;

uint8_t rx_buffer[16];
uint8_t rx_buffer_ready = 0; //0 when no data read (not ready for reading), 1 when data received (ready for reading)

uint8_t tx_buffer[16];
uint8_t tx_buffer_ready = 1; //1 when ready, 0 when writing is in progress

uint32_t last_heartbeat_time = 0;
uint32_t timeout_duration;

HAL_StatusTypeDef nrf_init(const uint32_t timeout)
{
	timeout_duration = timeout;
	return HAL_UART_Receive_DMA(&huart2, rx_buffer, sizeof(rx_buffer));
}

HAL_StatusTypeDef nrf_write(const uint8_t id, const uint8_t* buffer, const uint8_t size)
{
	while (tx_buffer_ready != 1)
		HAL_Delay(1);

	if (size == 0)
		memset(tx_buffer, 0, sizeof(tx_buffer));
	else
		memcpy(tx_buffer + 1, buffer, size);

	tx_buffer[0] = id;

	tx_buffer_ready = 0;
	return HAL_UART_Transmit_IT(&huart2, tx_buffer, sizeof(tx_buffer));
}

uint8_t nrf_available()
{
	return rx_buffer_ready;
}

const uint8_t* nrf_rx_buffer()
{
	rx_buffer_ready = 0;
	return rx_buffer;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart != &huart2)
		return;

	rx_buffer_ready = 1;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart != &huart2)
		return;

	tx_buffer_ready = 1;
}

uint8_t nrf_timeout()
{
	if ((HAL_GetTick() - last_heartbeat_time) >= timeout_duration)
		return 1;

	return 0;
}

void nrf_update_heartbeat()
{
	last_heartbeat_time = HAL_GetTick();
}
