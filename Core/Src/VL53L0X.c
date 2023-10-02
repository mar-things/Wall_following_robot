#include "VL53L0X.h"

#define SIGNAL_RATE 0.25f
#define TIMING_BUDGET 50000

#define calcMacroPeriod(vcsel_period_pclks) ((((uint32_t)2304 * (vcsel_period_pclks) * 1655) + 500) / 1000)
#define encodeVcselPeriod(period_pclks) (((period_pclks) >> 1) - 1)
#define decodeVcselPeriod(reg_val)      (((reg_val) + 1) << 1)

typedef enum { VcselPeriodPreRange, VcselPeriodFinalRange } vcselPeriodType;

typedef struct {
  uint8_t tcc, msrc, dss, pre_range, final_range;
}SequenceStepEnables;

typedef struct {
  uint16_t pre_range_vcsel_period_pclks, final_range_vcsel_period_pclks;

  uint16_t msrc_dss_tcc_mclks, pre_range_mclks, final_range_mclks;
  uint32_t msrc_dss_tcc_us,    pre_range_us,    final_range_us;
}SequenceStepTimeouts;

HAL_StatusTypeDef laser_get_spad(laser_handle* laser, uint8_t* count, uint8_t* type_is_aperture);
uint32_t timeoutMclksToMicroseconds(uint16_t timeout_period_mclks, uint8_t vcsel_period_pclks);
HAL_StatusTypeDef laser_set_signal_rate(laser_handle* laser, float limit);
HAL_StatusTypeDef laser_set_timing_budget(laser_handle* laser, uint32_t budget_us);
HAL_StatusTypeDef laser_get_timing_budget(laser_handle* laser, uint32_t* budget);
HAL_StatusTypeDef laser_get_pulse_period(laser_handle* laser, vcselPeriodType type, uint8_t* period);

enum registers {
  SYSRANGE_START                              = 0x00,

  SYSTEM_THRESH_HIGH                          = 0x0C,
  SYSTEM_THRESH_LOW                           = 0x0E,

  SYSTEM_SEQUENCE_CONFIG                      = 0x01,
  SYSTEM_RANGE_CONFIG                         = 0x09,
  SYSTEM_INTERMEASUREMENT_PERIOD              = 0x04,

  SYSTEM_INTERRUPT_CONFIG_GPIO                = 0x0A,

  GPIO_HV_MUX_ACTIVE_HIGH                     = 0x84,

  SYSTEM_INTERRUPT_CLEAR                      = 0x0B,

  RESULT_INTERRUPT_STATUS                     = 0x13,
  RESULT_RANGE_STATUS                         = 0x14,

  RESULT_CORE_AMBIENT_WINDOW_EVENTS_RTN       = 0xBC,
  RESULT_CORE_RANGING_TOTAL_EVENTS_RTN        = 0xC0,
  RESULT_CORE_AMBIENT_WINDOW_EVENTS_REF       = 0xD0,
  RESULT_CORE_RANGING_TOTAL_EVENTS_REF        = 0xD4,
  RESULT_PEAK_SIGNAL_RATE_REF                 = 0xB6,

  ALGO_PART_TO_PART_RANGE_OFFSET_MM           = 0x28,

  I2C_SLAVE_DEVICE_ADDRESS                    = 0x8A,

  MSRC_CONFIG_CONTROL                         = 0x60,

  PRE_RANGE_CONFIG_MIN_SNR                    = 0x27,
  PRE_RANGE_CONFIG_VALID_PHASE_LOW            = 0x56,
  PRE_RANGE_CONFIG_VALID_PHASE_HIGH           = 0x57,
  PRE_RANGE_MIN_COUNT_RATE_RTN_LIMIT          = 0x64,

  FINAL_RANGE_CONFIG_MIN_SNR                  = 0x67,
  FINAL_RANGE_CONFIG_VALID_PHASE_LOW          = 0x47,
  FINAL_RANGE_CONFIG_VALID_PHASE_HIGH         = 0x48,
  FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT = 0x44,

  PRE_RANGE_CONFIG_SIGMA_THRESH_HI            = 0x61,
  PRE_RANGE_CONFIG_SIGMA_THRESH_LO            = 0x62,

  PRE_RANGE_CONFIG_VCSEL_PERIOD               = 0x50,
  PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI          = 0x51,
  PRE_RANGE_CONFIG_TIMEOUT_MACROP_LO          = 0x52,

  SYSTEM_HISTOGRAM_BIN                        = 0x81,
  HISTOGRAM_CONFIG_INITIAL_PHASE_SELECT       = 0x33,
  HISTOGRAM_CONFIG_READOUT_CTRL               = 0x55,

  FINAL_RANGE_CONFIG_VCSEL_PERIOD             = 0x70,
  FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI        = 0x71,
  FINAL_RANGE_CONFIG_TIMEOUT_MACROP_LO        = 0x72,
  CROSSTALK_COMPENSATION_PEAK_RATE_MCPS       = 0x20,

  MSRC_CONFIG_TIMEOUT_MACROP                  = 0x46,

  SOFT_RESET_GO2_SOFT_RESET_N                 = 0xBF,
  IDENTIFICATION_MODEL_ID                     = 0xC0,
  IDENTIFICATION_REVISION_ID                  = 0xC2,

  OSC_CALIBRATE_VAL                           = 0xF8,

  GLOBAL_CONFIG_VCSEL_WIDTH                   = 0x32,
  GLOBAL_CONFIG_SPAD_ENABLES_REF_0            = 0xB0,
  GLOBAL_CONFIG_SPAD_ENABLES_REF_1            = 0xB1,
  GLOBAL_CONFIG_SPAD_ENABLES_REF_2            = 0xB2,
  GLOBAL_CONFIG_SPAD_ENABLES_REF_3            = 0xB3,
  GLOBAL_CONFIG_SPAD_ENABLES_REF_4            = 0xB4,
  GLOBAL_CONFIG_SPAD_ENABLES_REF_5            = 0xB5,

  GLOBAL_CONFIG_REF_EN_START_SELECT           = 0xB6,
  DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD         = 0x4E,
  DYNAMIC_SPAD_REF_EN_START_OFFSET            = 0x4F,
  POWER_MANAGEMENT_GO1_POWER_FORCE            = 0x80,

  VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV           = 0x89,

  ALGO_PHASECAL_LIM                           = 0x30,
  ALGO_PHASECAL_CONFIG_TIMEOUT                = 0x30,
};

void reverse_bytes(uint8_t* data, const uint8_t size)
{
	for (uint8_t i = 0; i < size / 2; ++i)
	{
		const uint8_t tmp = data[i];
		data[i] = data[size - 1 - i];
		data[size - 1 - i] = tmp;
	}
}

HAL_StatusTypeDef laser_write(laser_handle* laser, uint8_t reg, uint8_t* data, const uint8_t size)
{
	//Convert LSB first to MSB first
	reverse_bytes(data, size);

	const HAL_StatusTypeDef status = HAL_I2C_Mem_Write(laser->hi2c,
													   laser->address,
													   reg,
													   sizeof(reg),
													   data,
													   size,
													   laser->timeout);

	//Convert MSB first to LSB first
	reverse_bytes(data, size);

	return status;
}

HAL_StatusTypeDef laser_read(laser_handle* laser, uint8_t reg, uint8_t* data, const uint8_t size)
{
	//Select register
	HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(laser->hi2c,
													   laser->address,
													   &reg,
													   sizeof(reg),
													   laser->timeout);

	if (status != HAL_OK)
		return status;

	//Read register contents
	status = HAL_I2C_Master_Receive(laser->hi2c,
									laser->address + 1,
									data,
									size,
									laser->timeout);

	if (status == HAL_OK)
	{
		//Convert MSB first to LSB first
		reverse_bytes(data, size);
	}

	return status;
}

HAL_StatusTypeDef laser_write_u8(laser_handle* laser, uint8_t reg, uint8_t data)
{
	return laser_write(laser, reg, &data, sizeof(data));
}

HAL_StatusTypeDef laser_write_u16(laser_handle* laser, uint8_t reg, uint16_t data)
{
	return laser_write(laser, reg, (uint8_t*)&data, sizeof(data));
}

HAL_StatusTypeDef laser_read_u8(laser_handle* laser, uint8_t reg, uint8_t* data)
{
	return laser_read(laser, reg, data, sizeof(uint8_t));
}

HAL_StatusTypeDef laser_read_u16(laser_handle* laser, uint8_t reg, uint16_t* data)
{
	return laser_read(laser, reg, (uint8_t*)data, sizeof(uint16_t));
}

HAL_StatusTypeDef laser_calibrate(laser_handle* laser, uint8_t vhv_init_byte)
{
	HAL_StatusTypeDef status;
	status = laser_write_u8(laser, SYSRANGE_START, 0x01 | vhv_init_byte);
	if (status != HAL_OK)
		return status;

	uint8_t tmp;
	const uint32_t tick = HAL_GetTick();
	do
	{
	  status = laser_read_u8(laser, RESULT_INTERRUPT_STATUS, &tmp);
	  if (status != HAL_OK)
		return status;

	  if (HAL_GetTick() - tick >= laser->timeout)
		  return HAL_TIMEOUT;

	} while (tmp == 0);

	status = laser_write_u8(laser, SYSTEM_INTERRUPT_CLEAR, 0x01);
	if (status != HAL_OK)
		return status;

	return laser_write_u8(laser, SYSRANGE_START, 0x00);
}

HAL_StatusTypeDef laser_init(laser_handle* laser)
{
	HAL_StatusTypeDef status;

	// "Set I2C standard mode"
	status = laser_write_u8(laser, 0x88, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x80, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x00, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_read_u8(laser, 0x91, &laser->g_stop);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x00, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x80, 0x00);
	if (status != HAL_OK)
		return status;

	// disable SIGNAL_RATE_MSRC (bit 1) and SIGNAL_RATE_PRE_RANGE (bit 4) limit checks
	uint8_t msrc_ctrl;
	status = laser_read_u8(laser, MSRC_CONFIG_CONTROL, &msrc_ctrl);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, MSRC_CONFIG_CONTROL, msrc_ctrl | 0x12);
	if (status != HAL_OK)
		return status;

	// set final range signal rate limit to 0.25 MCPS (million counts per second)
	status = laser_set_signal_rate(laser, SIGNAL_RATE);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, SYSTEM_SEQUENCE_CONFIG, 0xFF);
	if (status != HAL_OK)
		return status;

	// VL53L0X_DataInit() end

	// VL53L0X_StaticInit() begin

	uint8_t spad_count;
	uint8_t spad_type_is_aperture;
	status = laser_get_spad(laser, &spad_count, &spad_type_is_aperture);
	if (status != HAL_OK)
		return status;

	// The SPAD map (RefGoodSpadMap) is read by VL53L0X_get_info_from_device() in
	// the API, but the same data seems to be more easily readable from
	// GLOBAL_CONFIG_SPAD_ENABLES_REF_0 through _6, so read it from there
	uint8_t ref_spad_map[6];
	status = laser_read(laser, GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);
	if (status != HAL_OK)
		return status;
	// -- VL53L0X_set_reference_spads() begin (assume NVM values are valid)

	status = laser_write_u8(laser, 0xFF, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, DYNAMIC_SPAD_REF_EN_START_OFFSET, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD, 0x2C);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, GLOBAL_CONFIG_REF_EN_START_SELECT, 0xB4);
	if (status != HAL_OK)
		return status;

	uint8_t first_spad_to_enable = spad_type_is_aperture ? 12 : 0; // 12 is the first aperture spad
	uint8_t spads_enabled = 0;

	for (uint8_t i = 0; i < 48; i++)
	{
	if (i < first_spad_to_enable || spads_enabled == spad_count)
	{
	  // This bit is lower than the first one that should be enabled, or
	  // (reference_spad_count) bits have already been enabled, so zero this bit
	  ref_spad_map[i / 8] &= ~(1 << (i % 8));
	}
	else if ((ref_spad_map[i / 8] >> (i % 8)) & 0x1)
	{
	  spads_enabled++;
	}
	}

	status = laser_write(laser, GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);
	if (status != HAL_OK)
		return status;
	// -- VL53L0X_set_reference_spads() end

	// -- VL53L0X_load_tuning_settings() begin
	// DefaultTuningSettings from vl53l0x_tuning.h

	status = laser_write_u8(laser, 0xFF, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x00, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x09, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x10, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x11, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x24, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x25, 0xFF);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x75, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x4E, 0x2C);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x48, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x30, 0x20);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x30, 0x09);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x54, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x31, 0x04);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x32, 0x03);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x40, 0x83);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x46, 0x25);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x60, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x27, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x50, 0x06);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x51, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x52, 0x96);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x56, 0x08);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x57, 0x30);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x61, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x62, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x64, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x65, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x66, 0xA0);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x22, 0x32);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x47, 0x14);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x49, 0xFF);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x4A, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x7A, 0x0A);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x7B, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x78, 0x21);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x23, 0x34);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x42, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x44, 0xFF);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x45, 0x26);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x46, 0x05);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x40, 0x40);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x0E, 0x06);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x20, 0x1A);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x43, 0x40);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x34, 0x03);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x35, 0x44);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x31, 0x04);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x4B, 0x09);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x4C, 0x05);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x4D, 0x04);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x44, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x45, 0x20);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x47, 0x08);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x48, 0x28);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x67, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x70, 0x04);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x71, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x72, 0xFE);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x76, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x77, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x0D, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x80, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x01, 0xF8);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x8E, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x00, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x80, 0x00);
	if (status != HAL_OK)
		return status;


	// -- VL53L0X_load_tuning_settings() end

	// "Set interrupt config to new sample ready"
	// -- VL53L0X_SetGpioConfig() begin

	status = laser_write_u8(laser, SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04);
	if (status != HAL_OK)
		return status;

	uint8_t tmp;
	status = laser_read_u8(laser, GPIO_HV_MUX_ACTIVE_HIGH, &tmp);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, GPIO_HV_MUX_ACTIVE_HIGH, tmp & ~0x10); // active low
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, SYSTEM_INTERRUPT_CLEAR, 0x01);
	if (status != HAL_OK)
		return status;

	// -- VL53L0X_SetGpioConfig() end

	/*uint32_t budget;
	status = laser_get_timing_budget(laser, &budget);
	if (status != HAL_OK)
		return status;*/

	// "Disable MSRC and TCC by default"
	// MSRC = Minimum Signal Rate Check
	// TCC = Target CentreCheck
	// -- VL53L0X_SetSequenceStepEnable() begin

	status = laser_write_u8(laser, SYSTEM_SEQUENCE_CONFIG, 0xE8);
	if (status != HAL_OK)
		return status;

	// -- VL53L0X_SetSequenceStepEnable() end

	// "Recalculate timing budget"
	status = laser_set_timing_budget(laser, TIMING_BUDGET);
	if (status != HAL_OK)
		return status;

	// VL53L0X_StaticInit() end

	// VL53L0X_PerformRefCalibration() begin (VL53L0X_perform_ref_calibration())

	// -- VL53L0X_perform_vhv_calibration() begin

	status = laser_write_u8(laser, SYSTEM_SEQUENCE_CONFIG, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_calibrate(laser, 0x40);
	if (status != HAL_OK)
		return status;

	// -- VL53L0X_perform_vhv_calibration() end

	// -- VL53L0X_perform_phase_calibration() begin

	status = laser_write_u8(laser, SYSTEM_SEQUENCE_CONFIG, 0x02);
	if (status != HAL_OK)
		return status;

	status = laser_calibrate(laser, 0x00);
	if (status != HAL_OK)
		return status;

	// -- VL53L0X_perform_phase_calibration() end

	// "restore the previous Sequence Config"
	status = laser_write_u8(laser, SYSTEM_SEQUENCE_CONFIG, 0xE8);
	// VL53L0X_PerformRefCalibration() end

	return status;
}

HAL_StatusTypeDef laser_set_signal_rate(laser_handle* laser, float limit)
{
	if (limit < 0.f || limit > 511.99f)
		return HAL_ERROR;

	// Q9.7 fixed point format (9 integer bits, 7 fractional bits)
	return laser_write_u16(laser, FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, limit * (1 << 7));
}

HAL_StatusTypeDef laser_get_signal_rate(laser_handle* laser, float* limit)
{
	HAL_StatusTypeDef status;
	uint16_t rate;

	status = laser_read_u16(laser, FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, &rate);

	if (status == HAL_OK)
		*limit = (float)rate / ((float)(1 << 7));

	return status;
}

HAL_StatusTypeDef laser_get_spad(laser_handle* laser, uint8_t* count, uint8_t* type_is_aperture)
{
	HAL_StatusTypeDef status;
	uint8_t tmp;

	status = laser_write_u8(laser, 0x80, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x00, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x06);
	if (status != HAL_OK)
		return status;

	status = laser_read_u8(laser, 0x83, &tmp);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x83, tmp | 0x04);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x07);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x81, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x80, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x94, 0x6b);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x83, 0x00);
	if (status != HAL_OK)
		return status;

	const uint32_t ticks = HAL_GetTick();
	do
	{
		status = laser_read_u8(laser, 0x83, &tmp);
		if (status != HAL_OK)
			return status;

		if (HAL_GetTick() - ticks >= laser->timeout)
			return HAL_TIMEOUT;
	} while (tmp == 0x00);

	status = laser_write_u8(laser, 0x83, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x83, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_read_u8(laser, 0x92, &tmp);
	if (status != HAL_OK)
		return status;

	*count = tmp & 0x7f;
	*type_is_aperture = (tmp >> 7) & 0x01;

	status = laser_write_u8(laser, 0x81, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x06);
	if (status != HAL_OK)
		return status;

	status = laser_read_u8(laser, 0x83, &tmp);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x83, tmp  & ~0x04);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x00, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x80, 0x00);
	if (status != HAL_OK)
		return status;

	return HAL_OK;
}

HAL_StatusTypeDef laser_get_seq_en(laser_handle* laser, SequenceStepEnables * enables)
{
	uint8_t sequence_config;
	HAL_StatusTypeDef status = laser_read_u8(laser, SYSTEM_SEQUENCE_CONFIG, &sequence_config);

	if (status == HAL_OK)
	{
		enables->tcc          = (sequence_config >> 4) & 0x1;
		enables->dss          = (sequence_config >> 3) & 0x1;
		enables->msrc         = (sequence_config >> 2) & 0x1;
		enables->pre_range    = (sequence_config >> 6) & 0x1;
		enables->final_range  = (sequence_config >> 7) & 0x1;
	}

	return status;
}

uint32_t laser_us_to_mclks(uint16_t timeout_period_us, uint8_t vcsel_period_pclks)
{
	const uint32_t macro_period_ns = calcMacroPeriod(vcsel_period_pclks);
	return (((timeout_period_us * 1000) + (macro_period_ns / 2)) / macro_period_ns);
}

uint32_t laser_mclks_to_us(uint16_t timeout_period_mclks, uint8_t vcsel_period_pclks)
{
	const uint32_t macro_period_ns = calcMacroPeriod(vcsel_period_pclks);
	return ((timeout_period_mclks * macro_period_ns) + (macro_period_ns / 2)) / 1000;
}

uint16_t decodeTimeout(uint16_t reg_val)
{
  // format: "(LSByte * 2^MSByte) + 1"
  return (uint16_t)((reg_val & 0x00FF) <<
         (uint16_t)((reg_val & 0xFF00) >> 8)) + 1;
}

HAL_StatusTypeDef laser_get_seq_timeouts(laser_handle* laser, SequenceStepEnables const * enables, SequenceStepTimeouts * timeouts)
{
  HAL_StatusTypeDef status;

  status = laser_get_pulse_period(laser, VcselPeriodPreRange, (uint8_t*)&timeouts->pre_range_vcsel_period_pclks);
  if (status != HAL_OK)
	  return status;

  status = laser_read_u8(laser, MSRC_CONFIG_TIMEOUT_MACROP, (uint8_t*)&timeouts->msrc_dss_tcc_mclks);
  if (status != HAL_OK)
  	  return status;
  timeouts->msrc_dss_tcc_mclks++;

  timeouts->msrc_dss_tcc_us =
		  laser_mclks_to_us(timeouts->msrc_dss_tcc_mclks,
                               timeouts->pre_range_vcsel_period_pclks);

  uint16_t tmp;
  status = laser_read_u16(laser, PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI, &tmp);
  if (status != HAL_OK)
	  return status;
  timeouts->pre_range_mclks = decodeTimeout(tmp);

  timeouts->pre_range_us =
		  laser_mclks_to_us(timeouts->pre_range_mclks,
                               timeouts->pre_range_vcsel_period_pclks);

  status = laser_get_pulse_period(laser, VcselPeriodFinalRange, (uint8_t*)&timeouts->final_range_vcsel_period_pclks);
  if (status != HAL_OK)
	  return status;

  status = laser_read_u16(laser, FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, &tmp);
  if (status != HAL_OK)
	  return status;

  timeouts->final_range_mclks = decodeTimeout(tmp);

  if (enables->pre_range)
  {
    timeouts->final_range_mclks -= timeouts->pre_range_mclks;
  }

  timeouts->final_range_us =
		  laser_mclks_to_us(timeouts->final_range_mclks,
                               timeouts->final_range_vcsel_period_pclks);

  return HAL_OK;
}

HAL_StatusTypeDef laser_get_pulse_period(laser_handle* laser, vcselPeriodType type, uint8_t* period)
{
	HAL_StatusTypeDef status;
  if (type == VcselPeriodPreRange)
  {
	  status = laser_read_u8(laser, PRE_RANGE_CONFIG_VCSEL_PERIOD, period);
	  if (status == HAL_OK)
		  *period = decodeVcselPeriod(*period);
	  return status;
  }
  else if (type == VcselPeriodFinalRange)
  {
	status = laser_read_u8(laser, FINAL_RANGE_CONFIG_VCSEL_PERIOD, period);
	if (status == HAL_OK)
		*period = decodeVcselPeriod(*period);
	return status;
  }

  return HAL_ERROR;
}

HAL_StatusTypeDef laser_read_continuous(laser_handle* laser, uint16_t* mm)
{
	HAL_StatusTypeDef status;

	uint8_t tmp;

	const uint32_t tick = HAL_GetTick();
	do
	{
		status = laser_read_u8(laser, RESULT_INTERRUPT_STATUS, &tmp);
		if (status != HAL_OK)
			return status;

		if (HAL_GetTick() - tick >= laser->timeout)
			return HAL_TIMEOUT;

	} while (tmp == 0);

	status = laser_read_u16(laser, RESULT_RANGE_STATUS + 10, mm);;

	return laser_write_u8(laser, SYSTEM_INTERRUPT_CLEAR, 0x01);
}

HAL_StatusTypeDef laser_get_timing_budget(laser_handle* laser, uint32_t* budget_us)
{
  SequenceStepEnables enables;
  SequenceStepTimeouts timeouts;

  uint16_t const StartOverhead     = 1910; // note that this is different than the value in set_
  uint16_t const EndOverhead        = 960;
  uint16_t const MsrcOverhead       = 660;
  uint16_t const TccOverhead        = 590;
  uint16_t const DssOverhead        = 690;
  uint16_t const PreRangeOverhead   = 660;
  uint16_t const FinalRangeOverhead = 550;

  // "Start and end overhead times always present"
  *budget_us = StartOverhead + EndOverhead;

  HAL_StatusTypeDef status = laser_get_seq_en(laser, &enables);
  if (status != HAL_OK)
	  return status;

  status = laser_get_seq_timeouts(laser, &enables, &timeouts);
  if (status != HAL_OK)
  	  return status;

  if (enables.tcc)
  {
    *budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead);
  }

  if (enables.dss)
  {
    *budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
  }
  else if (enables.msrc)
  {
    *budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead);
  }

  if (enables.pre_range)
  {
    *budget_us += (timeouts.pre_range_us + PreRangeOverhead);
  }

  if (enables.final_range)
  {
    *budget_us += (timeouts.final_range_us + FinalRangeOverhead);
  }

  return HAL_OK;
}

uint16_t encodeTimeout(uint16_t timeout_mclks)
{
  // format: "(LSByte * 2^MSByte) + 1"

  uint32_t ls_byte = 0;
  uint16_t ms_byte = 0;

  if (timeout_mclks > 0)
  {
    ls_byte = timeout_mclks - 1;

    while ((ls_byte & 0xFFFFFF00) > 0)
    {
      ls_byte >>= 1;
      ms_byte++;
    }

    return (ms_byte << 8) | (ls_byte & 0xFF);
  }
  else { return 0; }
}

HAL_StatusTypeDef laser_set_timing_budget(laser_handle* laser, uint32_t budget_us)
{
	HAL_StatusTypeDef status;
	SequenceStepEnables enables;
	SequenceStepTimeouts timeouts;

	uint16_t const StartOverhead      = 1320; // note that this is different than the value in get_
	uint16_t const EndOverhead        = 960;
	uint16_t const MsrcOverhead       = 660;
	uint16_t const TccOverhead        = 590;
	uint16_t const DssOverhead        = 690;
	uint16_t const PreRangeOverhead   = 660;
	uint16_t const FinalRangeOverhead = 550;

	uint32_t const MinTimingBudget = 20000;

	if (budget_us < MinTimingBudget) { return HAL_ERROR; }

	uint32_t used_budget_us = StartOverhead + EndOverhead;

	status = laser_get_seq_en(laser, &enables);
	if (status != HAL_OK)
		return status;
	status = laser_get_seq_timeouts(laser, &enables, &timeouts);
	if (status != HAL_OK)
		return status;

	if (enables.tcc)
	{
		used_budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead);
	}

	if (enables.dss)
	{
		used_budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
	}
	else if (enables.msrc)
	{
		used_budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead);
	}

	if (enables.pre_range)
	{
		used_budget_us += (timeouts.pre_range_us + PreRangeOverhead);
	}

	if (enables.final_range)
	{
		used_budget_us += FinalRangeOverhead;

		// "Note that the final range timeout is determined by the timing
		// budget and the sum of all other timeouts within the sequence.
		// If there is no room for the final range timeout, then an error
		// will be set. Otherwise the remaining time will be applied to
		// the final range."

		if (used_budget_us > budget_us)
		{
		  // "Requested timeout too big."
		  return HAL_ERROR;
		}

		uint32_t final_range_timeout_us = budget_us - used_budget_us;

		// set_sequence_step_timeout() begin
		// (SequenceStepId == VL53L0X_SEQUENCESTEP_FINAL_RANGE)

		// "For the final range timeout, the pre-range timeout
		//  must be added. To do this both final and pre-range
		//  timeouts must be expressed in macro periods MClks
		//  because they have different vcsel periods."

		uint16_t final_range_timeout_mclks =
				laser_us_to_mclks(final_range_timeout_us,
									 timeouts.final_range_vcsel_period_pclks);

		if (enables.pre_range)
		{
		  final_range_timeout_mclks += timeouts.pre_range_mclks;
		}

		status = laser_write_u16(laser, FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, encodeTimeout(final_range_timeout_mclks));


		// set_sequence_step_timeout() end
	}

	return HAL_OK;
}

HAL_StatusTypeDef laser_start_continuous(laser_handle* laser)
{
	uint32_t period_ms = 0; //BACK to BACK mode, can be passed as parameter

	HAL_StatusTypeDef status;

	status = laser_write_u8(laser, 0x80, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x00, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x91, laser->g_stop);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x00, 0x01);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0xFF, 0x00);
	if (status != HAL_OK)
		return status;

	status = laser_write_u8(laser, 0x80, 0x00);
	if (status != HAL_OK)
		return status;

  if (period_ms != 0)
  {
    // continuous timed mode

    // VL53L0X_SetInterMeasurementPeriodMilliSeconds() begin

    uint16_t osc_calibrate_val;
    status = laser_read_u16(laser, OSC_CALIBRATE_VAL, &osc_calibrate_val);
    if (status != HAL_OK)
		return status;

    if (osc_calibrate_val != 0)
    {
      period_ms *= osc_calibrate_val;
    }

    status = laser_write(laser, SYSTEM_INTERMEASUREMENT_PERIOD, (uint8_t*)&period_ms, sizeof(period_ms));
    if (status != HAL_OK)
		return status;

    // VL53L0X_SetInterMeasurementPeriodMilliSeconds() end

    status = laser_write_u8(laser, SYSRANGE_START, 0x04); // VL53L0X_REG_SYSRANGE_MODE_TIMED
  }
  else
  {
    // continuous back-to-back mode
    status = laser_write_u8(laser, SYSRANGE_START, 0x02); // VL53L0X_REG_SYSRANGE_MODE_BACKTOBACK
  }

  return status;
}

HAL_StatusTypeDef laser_set_address(laser_handle* laser, uint8_t new_address)
{
	const HAL_StatusTypeDef status = laser_write_u8(laser, I2C_SLAVE_DEVICE_ADDRESS, new_address >> 1);

	if (status == HAL_OK)
		laser->address = new_address;

	return status;
}
