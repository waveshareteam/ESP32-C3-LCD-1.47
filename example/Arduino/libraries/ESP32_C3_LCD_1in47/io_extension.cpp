#include "io_extension.h"

namespace {

constexpr uint32_t TimeoutMs = 20;

uint8_t scalePwm(uint8_t value)
{
  if (value > 100) {
    value = 100;
  }

  return (uint8_t)(value * 255 / 100);
}

esp_err_t io_extension_write_reg(io_extension_obj_t *obj, uint8_t reg, uint8_t value)
{
  uint8_t data[2] = {reg, value};

  if (obj == nullptr || obj->dev == nullptr) {
    return ESP_ERR_INVALID_STATE;
  }

  obj->err = i2c_master_transmit(obj->dev, data, sizeof(data), TimeoutMs);
  return obj->err;
}

esp_err_t io_extension_read_reg(io_extension_obj_t *obj, uint8_t reg, uint8_t *value)
{
  if (obj == nullptr || obj->dev == nullptr || value == nullptr) {
    return ESP_ERR_INVALID_STATE;
  }

  obj->err = i2c_master_transmit_receive(obj->dev, &reg, 1, value, 1, TimeoutMs);
  return obj->err;
}

esp_err_t io_extension_read_word(io_extension_obj_t *obj, uint8_t reg, uint16_t *value)
{
  uint8_t data[2] = {0};

  if (obj == nullptr || obj->dev == nullptr || value == nullptr) {
    return ESP_ERR_INVALID_STATE;
  }

  obj->err = i2c_master_transmit_receive(obj->dev, &reg, 1, data, sizeof(data), TimeoutMs);
  if (obj->err != ESP_OK) {
    return obj->err;
  }

  *value = (uint16_t)(data[1] << 8) | data[0];
  return ESP_OK;
}

}  // namespace

bool IO_EXTENSION_Init(io_extension_obj_t *obj, gpio_num_t sda, gpio_num_t scl, i2c_port_num_t port, uint32_t hz)
{
  i2c_master_bus_config_t bus_cfg = {};
  i2c_device_config_t dev_cfg = {};

  if (obj == nullptr) {
    return false;
  }

  if (obj->bus == nullptr) {
    bus_cfg.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_cfg.i2c_port = port;
    bus_cfg.sda_io_num = sda;
    bus_cfg.scl_io_num = scl;
    bus_cfg.flags.enable_internal_pullup = true;
    obj->err = i2c_new_master_bus(&bus_cfg, &obj->bus);
    if (obj->err != ESP_OK) {
      return false;
    }
  }

  if (obj->dev == nullptr) {
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address = IO_EXTENSION_ADDR;
    dev_cfg.scl_speed_hz = hz;
    dev_cfg.scl_wait_us = 0;
    dev_cfg.flags.disable_ack_check = 0;
    obj->err = i2c_master_bus_add_device(obj->bus, &dev_cfg, &obj->dev);
    if (obj->err != ESP_OK) {
      return false;
    }
  }

  obj->output = 0xFF;
  obj->err = IO_EXTENSION_IO_Mode(obj, 0xFF);
  if (obj->err != ESP_OK) {
    return false;
  }

  obj->err = IO_EXTENSION_Output_Bits(obj, obj->output);
  return obj->err == ESP_OK;
}

esp_err_t IO_EXTENSION_IO_Mode(io_extension_obj_t *obj, uint8_t value)
{
  return io_extension_write_reg(obj, IO_EXTENSION_REG_MODE, value);
}

esp_err_t IO_EXTENSION_Output(io_extension_obj_t *obj, uint8_t pin, uint8_t value)
{
  if (obj == nullptr || pin >= 8) {
    return ESP_ERR_INVALID_ARG;
  }

  if (value) {
    obj->output |= (uint8_t)(1U << pin);
  }
  if (!value) {
    obj->output &= (uint8_t)~(1U << pin);
  }

  return IO_EXTENSION_Output_Bits(obj, obj->output);
}

esp_err_t IO_EXTENSION_Output_Bits(io_extension_obj_t *obj, uint8_t value)
{
  if (obj == nullptr) {
    return ESP_ERR_INVALID_ARG;
  }

  obj->output = value;
  return io_extension_write_reg(obj, IO_EXTENSION_REG_OUTPUT, value);
}

esp_err_t IO_EXTENSION_Pwm_Output(io_extension_obj_t *obj, uint8_t value)
{
  return io_extension_write_reg(obj, IO_EXTENSION_REG_PWM, scalePwm(value));
}

uint8_t IO_EXTENSION_Input(io_extension_obj_t *obj, uint8_t pin)
{
  uint8_t value = 0;

  if (pin >= 8) {
    return 0;
  }
  if (io_extension_read_reg(obj, IO_EXTENSION_REG_INPUT, &value) != ESP_OK) {
    return 0;
  }

  return (uint8_t)((value >> pin) & 0x01);
}

uint16_t IO_EXTENSION_Adc_Input(io_extension_obj_t *obj)
{
  uint16_t value = 0;

  if (io_extension_read_word(obj, IO_EXTENSION_REG_ADC, &value) != ESP_OK) {
    return 0;
  }

  return value;
}

uint8_t IO_EXTENSION_Rtc_Int_Read(io_extension_obj_t *obj)
{
  uint8_t value = 0;

  if (io_extension_read_reg(obj, IO_EXTENSION_REG_RTC_INT, &value) != ESP_OK) {
    return 0;
  }

  return value;
}

esp_err_t IO_EXTENSION_Last_Error(io_extension_obj_t *obj)
{
  if (obj == nullptr) {
    return ESP_ERR_INVALID_ARG;
  }

  return obj->err;
}
