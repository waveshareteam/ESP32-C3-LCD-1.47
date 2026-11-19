#pragma once

#include <Arduino.h>
#include "driver/i2c_master.h"

typedef struct {
  i2c_master_bus_handle_t bus;
  i2c_master_dev_handle_t dev;
  uint8_t output;
  esp_err_t err;
} io_extension_obj_t;

#define IO_EXTENSION_ADDR            0x24

#define IO_EXTENSION_REG_MODE        0x02
#define IO_EXTENSION_REG_OUTPUT      0x03
#define IO_EXTENSION_REG_INPUT       0x04
#define IO_EXTENSION_REG_PWM         0x05
#define IO_EXTENSION_REG_ADC         0x06
#define IO_EXTENSION_REG_RTC_INT     0x07

#define IO_EXTENSION_IO_0            0x00
#define IO_EXTENSION_IO_1            0x01
#define IO_EXTENSION_IO_2            0x02
#define IO_EXTENSION_IO_3            0x03
#define IO_EXTENSION_IO_4            0x04
#define IO_EXTENSION_IO_5            0x05
#define IO_EXTENSION_IO_6            0x06
#define IO_EXTENSION_IO_7            0x07

bool IO_EXTENSION_Init(io_extension_obj_t *obj, gpio_num_t sda, gpio_num_t scl, i2c_port_num_t port, uint32_t hz);
esp_err_t IO_EXTENSION_IO_Mode(io_extension_obj_t *obj, uint8_t value);
esp_err_t IO_EXTENSION_Output(io_extension_obj_t *obj, uint8_t pin, uint8_t value);
esp_err_t IO_EXTENSION_Output_Bits(io_extension_obj_t *obj, uint8_t value);
esp_err_t IO_EXTENSION_Pwm_Output(io_extension_obj_t *obj, uint8_t value);
uint8_t IO_EXTENSION_Input(io_extension_obj_t *obj, uint8_t pin);
uint16_t IO_EXTENSION_Adc_Input(io_extension_obj_t *obj);
uint8_t IO_EXTENSION_Rtc_Int_Read(io_extension_obj_t *obj);
esp_err_t IO_EXTENSION_Last_Error(io_extension_obj_t *obj);
