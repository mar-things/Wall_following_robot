#ifndef INC_LASERS_H_
#define INC_LASERS_H_

//Initializes all lasers and prepares them for reading
//Return 0 on error, 1 on success
uint8_t lasers_init();

//Reads laser distances to lasers_mm
//Distance is in millimeters
//If reading was succesful returns HAL_OK or error otherwise
HAL_StatusTypeDef lasers_read(uint16_t* lasers_mm);

void lasers_offset(uint16_t* lasers, const int8_t* offsets);

void lasers_deinit();

#endif
