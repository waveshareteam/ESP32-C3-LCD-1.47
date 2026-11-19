#include "io_extension.h"
#include <stdbool.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

io_extension_obj_t IO_EXTENSION;
static i2c_master_bus_handle_t i2c_bus = NULL;
static bool i2c_bus_inited = false;
static i2c_master_dev_handle_t ext_dev = NULL;
static uint8_t ext_dev_addr = 0;

#define IO_EXTENSION_I2C_PORT     (0)
#define IO_EXTENSION_I2C_SCL      (3)
#define IO_EXTENSION_I2C_SDA      (4)
#define IO_EXTENSION_I2C_SPEED_HZ (400000)

static esp_err_t io_extension_write_reg(uint8_t reg, uint8_t value)
{
    uint8_t data[2] = {reg, value};
    return i2c_master_transmit(IO_EXTENSION.addr, data, sizeof(data), 20);
}

static esp_err_t io_extension_i2c_init(void)
{
    if (i2c_bus_inited && i2c_bus != NULL) {
        return ESP_OK;
    }

    i2c_master_bus_config_t i2c_bus_conf = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .sda_io_num = IO_EXTENSION_I2C_SDA,
        .scl_io_num = IO_EXTENSION_I2C_SCL,
        .i2c_port = IO_EXTENSION_I2C_PORT,
        .flags.enable_internal_pullup = true,
    };
    esp_err_t ret = i2c_new_master_bus(&i2c_bus_conf, &i2c_bus);
    if (ret != ESP_OK) {
        return ret;
    }

    i2c_bus_inited = true;
    return ESP_OK;
}

static esp_err_t io_extension_ensure(void)
{
    esp_err_t ret = io_extension_i2c_init();
    if (ret != ESP_OK) {
        return ret;
    }

    if (IO_EXTENSION.addr != NULL) {
        return ESP_OK;
    }

    if (i2c_bus == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = IO_EXTENSION_ADDR,
        .scl_speed_hz = IO_EXTENSION_I2C_SPEED_HZ,
        .scl_wait_us = 0,
        .flags = {
            .disable_ack_check = 0,
        },
    };

    ret = i2c_master_bus_add_device(i2c_bus, &dev_cfg, &IO_EXTENSION.addr);
    if (ret == ESP_OK) {
        ext_dev = IO_EXTENSION.addr;
        ext_dev_addr = IO_EXTENSION_ADDR;
    }
    return ret;
}

static esp_err_t io_extension_get_dev(uint8_t dev_addr, i2c_master_dev_handle_t *dev)
{
    esp_err_t ret = io_extension_i2c_init();
    if (ret != ESP_OK) {
        return ret;
    }
    if (dev_addr == IO_EXTENSION_ADDR) {
        ret = io_extension_ensure();
        if (ret != ESP_OK) {
            return ret;
        }
        *dev = IO_EXTENSION.addr;
        return ESP_OK;
    }
    if (ext_dev != NULL && ext_dev_addr == dev_addr) {
        *dev = ext_dev;
        return ESP_OK;
    }
    if (i2c_bus == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = dev_addr,
        .scl_speed_hz = IO_EXTENSION_I2C_SPEED_HZ,
        .scl_wait_us = 0,
        .flags = {
            .disable_ack_check = 0,
        },
    };
    ret = i2c_master_bus_add_device(i2c_bus, &dev_cfg, &ext_dev);
    if (ret != ESP_OK) {
        return ret;
    }
    ext_dev_addr = dev_addr;
    *dev = ext_dev;
    return ESP_OK;
}

/**
 * @brief Set the IO mode for the specified pins.
 * 
 * This function sets the specified pins to input or output mode by writing to the mode register.
 * 
 * @param pin An 8-bit value where each bit represents a pin (0 = input, 1 = output).
 */
esp_err_t IO_EXTENSION_IO_Mode(uint8_t pin)
{
    esp_err_t ret = io_extension_ensure();
    if (ret != ESP_OK) {
        return ret;
    }   
    return io_extension_write_reg(IO_EXTENSION_Mode, pin);
}

/**
 * @brief Initialize the IO_EXTENSION device.
 * 
 * This function configures the slave addresses for different registers of the
 * IO_EXTENSION chip via I2C, and sets the control flags for input/output modes.
 */
esp_err_t IO_EXTENSION_Init(void)
{
    esp_err_t ret = io_extension_ensure();
    if (ret != ESP_OK) {
        return ret;
    }
    ret = IO_EXTENSION_IO_Mode(0xFF);
    if (ret != ESP_OK) {
        return ret;
    }
    IO_EXTENSION.Last_io_value = 0xF7;
    IO_EXTENSION.Last_od_value = 0xF7;
    return io_extension_write_reg(IO_EXTENSION_IO_OUTPUT_ADDR, IO_EXTENSION.Last_io_value);
}

/**
 * @brief Set the value of the IO output pins on the IO_EXTENSION device.
 * 
 * This function writes an 8-bit value to the IO output register. The value
 * determines the high or low state of the pins.
 * 
 * @param pin The pin number to set (0-7).
 * @param value The value to set on the specified pin (0 = low, 1 = high).
 */
esp_err_t IO_EXTENSION_Output(uint8_t pin, uint8_t value)
{
    if (pin >= 8) {
        return ESP_ERR_INVALID_ARG;
    }
    if (value) {
        IO_EXTENSION.Last_io_value |= (1 << pin);
    } else {
        IO_EXTENSION.Last_io_value &= (~(1 << pin));
    }
    return io_extension_write_reg(IO_EXTENSION_IO_OUTPUT_ADDR, IO_EXTENSION.Last_io_value);
}

/**
 * @brief Set the PWM output value on the IO_EXTENSION device.
 * 
 * This function sets the PWM output value, which controls the duty cycle of the PWM signal.
 * The duty cycle is calculated based on the input value and the resolution (12 bits).
 * 
 * @param Value The input value to set the PWM duty cycle (0-100).
 */
esp_err_t IO_EXTENSION_Pwm_Output(uint8_t value)
{
    if (value >= 100) {
        value = 100;
    }
    uint8_t duty = value * (255 / 100.0);
    return io_extension_write_reg(IO_EXTENSION_PWM_ADDR, duty);
}

esp_err_t IO_EXTENSION_I2C_Read(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint8_t len)
{
    if (data == NULL || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    i2c_master_dev_handle_t dev = NULL;
    esp_err_t ret = io_extension_get_dev(dev_addr, &dev);
    if (ret != ESP_OK) {
        return ret;
    }
    return i2c_master_transmit_receive(dev, &reg, 1, data, len, 20);
}

esp_err_t IO_EXTENSION_I2C_Write(uint8_t dev_addr, uint8_t reg, const uint8_t *data, uint8_t len)
{
    if (len > 0 && data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    i2c_master_dev_handle_t dev = NULL;
    esp_err_t ret = io_extension_get_dev(dev_addr, &dev);
    if (ret != ESP_OK) {
        return ret;
    }
    uint8_t payload[17] = {0};
    if (len > sizeof(payload) - 1) {
        return ESP_ERR_INVALID_SIZE;
    }
    payload[0] = reg;
    if (len > 0) {
        memcpy(&payload[1], data, len);
    }
    return i2c_master_transmit(dev, payload, len + 1, 20);
}
