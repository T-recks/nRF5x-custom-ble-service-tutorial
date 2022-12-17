/**
Author: Sebastian Cardenas
 */

#include <stdio.h>
#include "boards.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"


#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

/* TWI instance ID. */

#define TWI_INSTANCE_ID     0 // create a ID constant
#define ADDR 0x44 /**< SHT31 Default Address */
#define SHT31_MEAS_HIGHREP_STRETCH                                             \
  0x2C06 /**< Measurement High Repeatability with Clock Stretch Enabled */
#define SHT31_MEAS_MEDREP_STRETCH                                              \
  0x2C0D /**< Measurement Medium Repeatability with Clock Stretch Enabled */
#define SHT31_MEAS_LOWREP_STRETCH                                              \
  0x2C10 /**< Measurement Low Repeatability with Clock Stretch Enabled*/
#define SHT31_MEAS_HIGHREP                                                     \
  0x2400 /**< Measurement High Repeatability with Clock Stretch Disabled */
#define SHT31_MEAS_MEDREP                                                      \
  0x240B /**< Measurement Medium Repeatability with Clock Stretch Disabled */
#define SHT31_MEAS_LOWREP                                                      \
  0x2416 /**< Measurement Low Repeatability with Clock Stretch Disabled */
#define SHT31_READSTATUS 0xF32D   /**< Read Out of Status Register */
#define SHT31_CLEARSTATUS 0x3041  /**< Clear Status */
#define SHT31_SOFTRESET 0x30A2    /**< Soft Reset */
#define SHT31_HEATEREN 0x306D     /**< Heater Enable */
#define SHT31_HEATERDIS 0x3066    /**< Heater Disable */
#define SHT31_REG_HEATER_BIT 0x0d /**< Status Register Heater Bit */

// create a handle which will point to TWI instance, in this case its TWI_0
static uint8_t m_custom_value = 0;
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);
float temp;
float hum;

// a function to initialize the twi(i2c)
void twi_init(void)
{
  ret_code_t err_code; // a variable to hold error code

// Create a struct with configurations and pass the values to these configurations.
  const nrf_drv_twi_config_t twi_config = {
    .scl                = 27, // scl connected to pin 22, you can change it to any other pin
    .sda                = 26, // sda connected to pin 23, you can change it to any other pin
    .frequency          = NRF_DRV_TWI_FREQ_100K, // set the communication speed to 100K, we can select 250k or 400k as well
    .interrupt_priority = APP_IRQ_PRIORITY_HIGH, // Interrupt priority is set to high, keep in mind to change it if you are using a soft-device
    .clear_bus_init     = false // automatic bus clearing 

  };

  err_code = nrf_drv_twi_init(&m_twi, &twi_config, NULL, NULL); // initialize the twi
  APP_ERROR_CHECK(err_code); // check if any error occured during initialization

  nrf_drv_twi_enable(&m_twi); // enable the twi comm so that its ready to communicate with the sensor

}
ret_code_t write(uint16_t command) {
  uint8_t cmd[2];
  cmd[0] = command >> 8;
  cmd[1] = command & 0xFF;
  return nrf_drv_twi_tx(&m_twi, ADDR, cmd, 2*sizeof(cmd), true);
}

static uint8_t crc8(const uint8_t *data, int len) {
  const uint8_t poly = 0x31;
  uint8_t crc = 0xFF;
  for(int j = len; j; --j) {
    crc ^= *data++;
    for(int i = 8; i; --i){
        crc = (crc & 0x80) ? (crc << 1) ^ poly : (crc << 1);
    }
  }
  return crc;
}

void reset(void){
    write(SHT31_SOFTRESET);
    nrf_delay_ms(10);
}

float temp_in_c(float data) {
   data = data / (65535.0f);
   return data*175 - 45;
}
float hum_in_rh(float data) {
   data = data / (65535.0f); 
   return 100*data;
}

bool readData(void) {
    uint8_t readbuffer[6];
    write(SHT31_MEAS_HIGHREP); //tell the sensor we want to read
    nrf_delay_ms(20); //give some time for the sensor to react
    nrf_drv_twi_rx(&m_twi, ADDR, &readbuffer, sizeof(readbuffer)); //read
    if (readbuffer[2] != crc8(readbuffer, 2) || readbuffer[5] != crc8(readbuffer + 3, 2))
        return false;
    int32_t tmp = (int32_t)(((uint32_t) readbuffer[0] << 8 | readbuffer[1]));
    tmp = (float) tmp;
    temp = temp_in_c(tmp);
    uint32_t hm = ((uint32_t) readbuffer[3] << 8 | readbuffer[4]);
    hm = (float) hm;
    hum = hum_in_rh(hm);
    return true;
}

uint16_t readStatus(void) {
    write(SHT31_READSTATUS);
    uint8_t data[3];
    nrf_drv_twi_rx(&m_twi, ADDR, &data, sizeof(data)); //read
    uint16_t stat = data[0];
    stat <<= 8;
    stat |= data[1];
    return stat;
}


bool begin(void) {
    reset();
    return readStatus() != 0xFFFF;
}

/**
 * @brief Function for main application entry.
 */
int main(void)
{
// initialize the Logger so that we can print msgs on the logger
  APP_ERROR_CHECK(NRF_LOG_INIT(NULL)); 
  NRF_LOG_DEFAULT_BACKENDS_INIT();

  twi_init(); // call the twi initialization function
  bool b = begin();
  if(b) {
      NRF_LOG_INFO("Successfully turned on sensor") 
  }  else {
      NRF_LOG_INFO("Sensor didn't turn on :(");
  }
  while(true){
      readData(); 
      NRF_LOG_INFO("Temp:" NRF_LOG_FLOAT_MARKER " Hum: " NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(temp), NRF_LOG_FLOAT(hum));
      nrf_delay_ms(100);
  }
  return 0;
}

/** @} */
