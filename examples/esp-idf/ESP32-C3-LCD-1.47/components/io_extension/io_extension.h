#ifndef __IO_EXTENSION_H
#define __IO_EXTENSION_H

#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

 /* 
  * IO EXTENSION GPIO control via I2C - Register and Command Definitions
  *
  *
  * Example usage:
  * 1. Set the working mode by writing to the register at address 0x24
  * 2. Send function commands to control the GPIO pins and modes
  */
 
 /* IO EXTENSION Function Register Addresses */
 #define IO_EXTENSION_ADDR          0x24  // Slave address for mode configuration register
 
 /* Mode control flags (from the chip manual) */
 #define IO_EXTENSION_Mode             0x02 // 
 #define IO_EXTENSION_IO_OUTPUT_ADDR   0x03 // 
 #define IO_EXTENSION_IO_INPUT_ADDR    0x04 // 
 #define IO_EXTENSION_PWM_ADDR         0x05 // 
 #define IO_EXTENSION_ADC_ADDR         0x06 // 
 #define IO_EXTENSION_RTC_INT_ADDR     0x07 // 
 
 /* Specific IO pin assignments */
 #define IO_EXTENSION_IO_0          0x00  // IO0 
 #define IO_EXTENSION_IO_1          0x01  // IO1 
 #define IO_EXTENSION_IO_2          0x02  // IO2 
 #define IO_EXTENSION_IO_3          0x03  // IO3 
 #define IO_EXTENSION_IO_4          0x04  // IO4 
 #define IO_EXTENSION_IO_5          0x05  // IO5 
 #define IO_EXTENSION_IO_6          0x06  // IO6 
 #define IO_EXTENSION_IO_7          0x07  // IO7 

#define IO_EXTENSION_LCD_CS        IO_EXTENSION_IO_0 // LCD CS
#define IO_EXTENSION_LCD_RST       IO_EXTENSION_IO_1 // LCD RST
#define IO_EXTENSION_SD_CS         IO_EXTENSION_IO_2 // SD CS
 
 /* Structure to represent the IO EXTENSION device */
 typedef struct _io_extension_obj_t {
     i2c_master_dev_handle_t addr;      // Handle for mode configuration
     uint8_t Last_io_value;
     uint8_t Last_od_value;
 } io_extension_obj_t;

/* Function declarations */
esp_err_t IO_EXTENSION_Init(void);
esp_err_t IO_EXTENSION_IO_Mode(uint8_t pin);
esp_err_t IO_EXTENSION_Output(uint8_t pin, uint8_t value);
esp_err_t IO_EXTENSION_Pwm_Output(uint8_t value);
esp_err_t IO_EXTENSION_I2C_Read(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint8_t len);
esp_err_t IO_EXTENSION_I2C_Write(uint8_t dev_addr, uint8_t reg, const uint8_t *data, uint8_t len);
 
 #endif  // __IO_EXTENSION_H
