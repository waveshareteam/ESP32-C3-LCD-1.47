#include "ESP32_C3_LCD_1in47.h"

namespace {

constexpr int32_t LCD_SPI_HZ = 80000000;

constexpr gpio_num_t PIN_SCL = GPIO_NUM_3;
constexpr gpio_num_t PIN_SDA = GPIO_NUM_4;
constexpr i2c_port_num_t I2C_PORT = I2C_NUM_0;
constexpr uint32_t I2C_HZ = 400000;

constexpr uint8_t PIN_LCD_CS = 0;
constexpr uint8_t PIN_LCD_RST = 1;
constexpr uint8_t PIN_SD_CS = 2;
constexpr uint8_t PIN_BOOT = 9;

constexpr uint8_t IO_OUTPUT_INIT = 0xF7;

constexpr int8_t LCD_DC = 8;
constexpr int8_t LCD_SCK = 7;
constexpr int8_t LCD_MOSI = 5;
constexpr int8_t LCD_MISO = 6;

constexpr int16_t LCD_W = 180;
constexpr int16_t LCD_H = 320;
constexpr uint8_t LCD_X0 = 30;

struct PanelInitCmd {
  uint8_t cmd;
  const uint8_t *data;
  uint8_t len;
  uint16_t delay_ms;
};

static uint16_t swap565(uint16_t v)
{
  return (uint16_t)((v << 8) | (v >> 8));
}

class LCD_SPI_Bus : public Arduino_ESP32SPI {
public:
  using Arduino_ESP32SPI::Arduino_ESP32SPI;

  void beginWrite() override
  {
    _pixelStream = false;
    Arduino_ESP32SPI::beginWrite();
  }

  void endWrite() override
  {
    _pixelStream = false;
    Arduino_ESP32SPI::endWrite();
  }

  void writeCommand(uint8_t c) override
  {
    _pixelStream = isPixelWriteCommand(c);
    Arduino_ESP32SPI::writeCommand(c);
  }

  void writeCommand16(uint16_t c) override
  {
    _pixelStream = isPixelWriteCommand((uint8_t)(c & 0x00FFU));
    Arduino_ESP32SPI::writeCommand16(c);
  }

  void write16(uint16_t d) override
  {
    Arduino_ESP32SPI::write16(swap565(d));
  }

  void writeRepeat(uint16_t p, uint32_t len) override
  {
    Arduino_ESP32SPI::writeRepeat(swap565(p), len);
  }

  void writePixels(uint16_t *data, uint32_t len) override
  {
    writeBytes((uint8_t *)data, len * 2U);
  }

  void writePixelsRaw(const uint16_t *data, uint32_t len)
  {
    Arduino_ESP32SPI::writeBytes((uint8_t *)data, len * 2U);
  }

  void writeBytes(uint8_t *data, uint32_t len) override
  {
    if (!_pixelStream || (len & 1U) != 0U) {
      Arduino_ESP32SPI::writeBytes(data, len);
      return;
    }

    uint8_t swapBuf[ByteSwapBufSize];

    while (true) {
      if (len == 0) {
        break;
      }

      uint32_t chunk = len;
      if (chunk > sizeof(swapBuf)) {
        chunk = (uint32_t)sizeof(swapBuf);
      }
      chunk &= ~1U;

      swapBytePairs16(swapBuf, data, chunk);

      Arduino_ESP32SPI::writeBytes(swapBuf, chunk);
      data += chunk;
      len -= chunk;
    }
  }

private:
  static constexpr uint8_t CmdRamWrite = 0x2C;
  static constexpr uint8_t CmdRamWriteContinue = 0x3C;
  static constexpr size_t ByteSwapBufSize = 256;

  static bool isPixelWriteCommand(uint8_t cmd)
  {
    return cmd == CmdRamWrite || cmd == CmdRamWriteContinue;
  }

  static void swapBytePairs16(uint8_t *dst, const uint8_t *src, uint32_t len)
  {
    uint32_t index = 0;
    while (index < len) {
      dst[index] = src[index + 1];
      dst[index + 1] = src[index];
      index += 2;
    }
  }

  bool _pixelStream = false;
};

class LCD_Panel : public Arduino_ST7789 {
public:
  using Arduino_ST7789::Arduino_ST7789;

  bool begin(int32_t speed = GFX_NOT_DEFINED) override
  {
    _override_datamode = SPI_MODE0;
    return Arduino_TFT::begin(speed);
  }

  void draw16bitRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h) override
  {
    drawRgbBitmapInternal(x, y, bitmap, w, h);
  }

  void draw16bitRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h) override
  {
    drawRgbBitmapInternal(x, y, bitmap, w, h);
  }

protected:
  void tftInit() override
  {
  }

private:
  void drawRgbBitmapInternal(int16_t x, int16_t y, const uint16_t *bitmap, int16_t w, int16_t h)
  {
    int16_t drawWidth = w;
    LCD_SPI_Bus *bus = (LCD_SPI_Bus *)_bus;

    if (((x + w - 1) < 0) || ((y + h - 1) < 0) || (x > _max_x) || (y > _max_y)) {
      return;
    }

    if ((y + h - 1) > _max_y) {
      h -= (y + h - 1) - _max_y;
    }
    if (y < 0) {
      bitmap -= y * w;
      h += y;
      y = 0;
    }
    if ((x + w - 1) > _max_x) {
      drawWidth -= (x + w - 1) - _max_x;
    }
    if (x < 0) {
      bitmap -= x;
      drawWidth += x;
      x = 0;
    }

    startWrite();
    writeAddrWindow(x, y, (uint16_t)drawWidth, (uint16_t)h);

    if (drawWidth < w) {
      int16_t row = 0;
      while (row < h) {
        bus->writePixelsRaw(bitmap, (uint32_t)drawWidth);
        bitmap += w;
        ++row;
      }
      endWrite();
      return;
    }

    bus->writePixelsRaw(bitmap, (uint32_t)w * (uint32_t)h);
    endWrite();
  }
};

constexpr uint8_t PANEL_MADCTL_0[] = {0x00};
constexpr uint8_t PANEL_PIXEL_FORMAT[] = {0x55};
constexpr uint8_t PANEL_B0[] = {0x00, 0xE8};
constexpr uint8_t PANEL_B2[] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
constexpr uint8_t PANEL_B7[] = {0x75};
constexpr uint8_t PANEL_BB[] = {0x1A};
constexpr uint8_t PANEL_C0[] = {0x80};
constexpr uint8_t PANEL_C2[] = {0x01, 0xFF};
constexpr uint8_t PANEL_C3[] = {0x13};
constexpr uint8_t PANEL_C4[] = {0x20};
constexpr uint8_t PANEL_C6[] = {0x0F};
constexpr uint8_t PANEL_D0[] = {0xA4, 0xA1};
constexpr uint8_t PANEL_E0[] = {0xD0, 0x0D, 0x14, 0x0D, 0x0D, 0x09, 0x38, 0x44, 0x4E, 0x3A, 0x17, 0x18, 0x2F, 0x30};
constexpr uint8_t PANEL_E1[] = {0xD0, 0x09, 0x0F, 0x08, 0x07, 0x14, 0x37, 0x44, 0x4D, 0x38, 0x15, 0x16, 0x2C, 0x2E};
constexpr uint8_t PANEL_MADCTL_1[] = {0x48};

constexpr PanelInitCmd PANEL_INIT_CMD[] = {
  {0x11, NULL, 0, 100},
  {0x36, PANEL_MADCTL_0, sizeof(PANEL_MADCTL_0), 0},
  {0x3A, PANEL_PIXEL_FORMAT, sizeof(PANEL_PIXEL_FORMAT), 0},
  {0xB0, PANEL_B0, sizeof(PANEL_B0), 0},
  {0xB2, PANEL_B2, sizeof(PANEL_B2), 0},
  {0xB7, PANEL_B7, sizeof(PANEL_B7), 0},
  {0xBB, PANEL_BB, sizeof(PANEL_BB), 0},
  {0xC0, PANEL_C0, sizeof(PANEL_C0), 0},
  {0xC2, PANEL_C2, sizeof(PANEL_C2), 0},
  {0xC3, PANEL_C3, sizeof(PANEL_C3), 0},
  {0xC4, PANEL_C4, sizeof(PANEL_C4), 0},
  {0xC6, PANEL_C6, sizeof(PANEL_C6), 0},
  {0xD0, PANEL_D0, sizeof(PANEL_D0), 0},
  {0xE0, PANEL_E0, sizeof(PANEL_E0), 0},
  {0xE1, PANEL_E1, sizeof(PANEL_E1), 0},
  {0x21, NULL, 0, 0},
  {0x29, NULL, 0, 0},
  {0x2C, NULL, 0, 20},
  {0x36, PANEL_MADCTL_1, sizeof(PANEL_MADCTL_1), 0},
};

}  // namespace

ESP32_C3_LCD_1in47::ESP32_C3_LCD_1in47(uint8_t rotation)
  : _rot(rotation),
    _err(ESP_OK),
    _io{},
    _bus(NULL),
    _gfx(NULL)
{
  _io.output = IO_OUTPUT_INIT;
  _io.err = ESP_OK;
}

void ESP32_C3_LCD_1in47::initBootKey(void)
{
  pinMode(PIN_BOOT, INPUT_PULLUP);
}

bool ESP32_C3_LCD_1in47::createDisplay(void)
{
  if (_bus == NULL) {
    _bus = new LCD_SPI_Bus(LCD_DC, -1, LCD_SCK, LCD_MOSI);
    if (_bus == NULL) {
      _err = ESP_ERR_NO_MEM;
      return false;
    }
  }

  if (_gfx == NULL) {
    _gfx = new LCD_Panel(_bus, GFX_NOT_DEFINED, _rot, false, LCD_W, LCD_H, LCD_X0, 0, LCD_X0, 0);
    if (_gfx == NULL) {
      _err = ESP_ERR_NO_MEM;
      return false;
    }
  }

  return true;
}

esp_err_t ESP32_C3_LCD_1in47::selectLcd(void)
{
  esp_err_t ret = IO_EXTENSION_Output(&_io, PIN_SD_CS, 1);

  if (ret != ESP_OK) {
    return ret;
  }
  delay(10);
  ret = IO_EXTENSION_Output(&_io, PIN_LCD_CS, 0);
  if (ret != ESP_OK) {
    return ret;
  }
  delay(10);
  return ESP_OK;
}

esp_err_t ESP32_C3_LCD_1in47::selectSD(void)
{
  esp_err_t ret = IO_EXTENSION_Output(&_io, PIN_LCD_CS, 1);

  if (ret != ESP_OK) {
    return ret;
  }
  delay(10);
  ret = IO_EXTENSION_Output(&_io, PIN_SD_CS, 0);
  if (ret != ESP_OK) {
    return ret;
  }
  delay(10);
  _err = ret;
  return ret;
}

esp_err_t ESP32_C3_LCD_1in47::resetLcd(void)
{
  esp_err_t ret = selectLcd();

  if (ret != ESP_OK) {
    return ret;
  }
  ret = IO_EXTENSION_Output(&_io, PIN_LCD_RST, 1);
  if (ret != ESP_OK) {
    return ret;
  }
  delay(20);
  ret = IO_EXTENSION_Output(&_io, PIN_LCD_RST, 0);
  if (ret != ESP_OK) {
    return ret;
  }
  delay(20);
  ret = IO_EXTENSION_Output(&_io, PIN_LCD_RST, 1);
  if (ret != ESP_OK) {
    return ret;
  }
  delay(120);
  return ESP_OK;
}

esp_err_t ESP32_C3_LCD_1in47::setBacklight(uint8_t value)
{
  if (value > 100) {
    value = 100;
  }

  _err = selectLcd();
  if (_err != ESP_OK) {
    return _err;
  }

  _err = IO_EXTENSION_Pwm_Output(&_io, value);
  return _err;
}

esp_err_t ESP32_C3_LCD_1in47::initIo(void)
{
  if (!IO_EXTENSION_Init(&_io, PIN_SDA, PIN_SCL, I2C_PORT, I2C_HZ)) {
    return IO_EXTENSION_Last_Error(&_io);
  }

  _io.output = IO_OUTPUT_INIT;
  return IO_EXTENSION_Output_Bits(&_io, _io.output);
}

void ESP32_C3_LCD_1in47::writeCmd(uint8_t cmd, const uint8_t *data, uint32_t len)
{
  _bus->writeCommand(cmd);
  if (len == 0) {
    return;
  }
  _bus->writeBytes((uint8_t *)data, len);
}

void ESP32_C3_LCD_1in47::initPanel(void)
{
  uint32_t index = 0;

  _bus->beginWrite();
  while (index < (sizeof(PANEL_INIT_CMD) / sizeof(PANEL_INIT_CMD[0]))) {
    const PanelInitCmd *item = &PANEL_INIT_CMD[index];

    writeCmd(item->cmd, item->data, item->len);
    if (item->delay_ms > 0) {
      _bus->endWrite();
      delay(item->delay_ms);
      _bus->beginWrite();
    }
    ++index;
  }
  _bus->endWrite();
}

bool ESP32_C3_LCD_1in47::begin(uint8_t brightness, int32_t speed)
{
  if (!createDisplay()) {
    return false;
  }

  _err = initIo();
  if (_err != ESP_OK) {
    return false;
  }

  _err = resetLcd();
  if (_err != ESP_OK) {
    return false;
  }

  if (!_gfx->begin(speed > 0 ? speed : LCD_SPI_HZ)) {
    _err = ESP_FAIL;
    return false;
  }

  initPanel();
  delay(20);
  initBootKey();
  setBacklight(brightness);
  return _err == ESP_OK;
}

bool ESP32_C3_LCD_1in47::beginSD(SPIClass &spi, uint8_t cs, uint32_t hz)
{
  spi.begin(LCD_SCK, LCD_MISO, LCD_MOSI, cs);
  _err = selectSD();
  if (_err != ESP_OK) {
    return false;
  }
  spi.beginTransaction(SPISettings(hz, MSBFIRST, SPI_MODE0));
  spi.endTransaction();
  _err = selectLcd();
  return _err == ESP_OK;
}

Arduino_GFX *ESP32_C3_LCD_1in47::gfx() const
{
  return _gfx;
}

esp_err_t ESP32_C3_LCD_1in47::selectLCD(void)
{
  _err = selectLcd();
  return _err;
}

bool ESP32_C3_LCD_1in47::bootPressed() const
{
  return digitalRead(PIN_BOOT) == LOW;
}

esp_err_t ESP32_C3_LCD_1in47::lastError() const
{
  return _err;
}
