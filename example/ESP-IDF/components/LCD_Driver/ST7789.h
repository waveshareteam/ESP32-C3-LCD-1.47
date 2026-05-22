#pragma once
#include <stdio.h>
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_err.h"

// LCD SPI GPIO
// Using SPI2 
#define LCD_HOST  SPI2_HOST

#define EXAMPLE_LCD_PIXEL_CLOCK_HZ     (80 * 1000 * 1000)

#define EXAMPLE_PIN_NUM_SCLK           7
#define EXAMPLE_PIN_NUM_MOSI           5
#define EXAMPLE_PIN_NUM_MISO           6
#define EXAMPLE_PIN_NUM_LCD_DC         8
// The pixel number in horizontal and vertical
#define EXAMPLE_LCD_H_RES              180
#define EXAMPLE_LCD_V_RES              320
#define LCD_X_GAP                      30
#define LCD_Y_GAP                      0
// Bit number used to represent command and parameter
#define EXAMPLE_LCD_CMD_BITS           8
#define EXAMPLE_LCD_PARAM_BITS         8

extern esp_lcd_panel_handle_t panel_handle;

void switch_bus_to_sd(void);
void switch_bus_to_lcd(void);
// void switch_bus_to_sd_image(void);
// void switch_bus_to_lcd_image(void);


void BK_Light(uint8_t Light);                   // Call this function to adjust the brightness of the backlight. The value of the parameter Light ranges from 0 to 100
esp_err_t LCD_Init(void);                     // Call this function to initialize the screen (must be called in the main function) !!!!!
esp_err_t spi_bus_init(void);
