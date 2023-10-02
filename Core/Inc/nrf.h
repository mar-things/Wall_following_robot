#ifndef INC_NRF_H_
#define INC_NRF_H_

#include "usart.h"

//Starts receiving data
//Returns HAL_OK on success or error on failure
HAL_StatusTypeDef nrf_init(const uint32_t timeout);

//Writes id and buffer to nrf via UART
//buffer size depends on the id
//Returns HAL_OK on success or error on failure
HAL_StatusTypeDef nrf_write(const uint8_t id, const uint8_t* buffer, const uint8_t size);

//Returns 1 if data for reading is available, 0 otherwise
uint8_t nrf_available();

//Returns pointer to rx_buffer
const uint8_t* nrf_rx_buffer();

//Returns 1 if the connections has timed out, 0 otherwise
uint8_t nrf_timeout();

//Call when received heartbeat to update internal timer
void nrf_update_heartbeat();

#endif
