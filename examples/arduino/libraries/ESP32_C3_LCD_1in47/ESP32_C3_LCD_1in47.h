#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <SPI.h>
#include "io_extension.h"

class ESP32_C3_LCD_1in47 {
public:
  explicit ESP32_C3_LCD_1in47(uint8_t rotation = 4);

  bool begin(uint8_t brightness = 100, int32_t speed = 80000000);
  bool beginSD(SPIClass &spi = SPI, uint8_t cs = 10, uint32_t hz = 1000000);
  Arduino_GFX *gfx() const;
  esp_err_t setBacklight(uint8_t value);
  esp_err_t selectLCD(void);
  esp_err_t selectSD(void);
  esp_err_t lastError() const;
  bool bootPressed() const;

private:
  bool createDisplay(void);
  void initBootKey(void);
  esp_err_t selectLcd(void);
  esp_err_t resetLcd(void);
  esp_err_t initIo(void);
  void writeCmd(uint8_t cmd, const uint8_t *data, uint32_t len);
  void initPanel(void);

  uint8_t _rot;
  esp_err_t _err;
  io_extension_obj_t _io;
  Arduino_DataBus *_bus;
  Arduino_GFX *_gfx;
};
