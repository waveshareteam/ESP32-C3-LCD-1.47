#include <ESP32_C3_LCD_1in47.h>
#include <JPEGDEC.h>
#include <SD.h>
#include <SPI.h>
#include "esp_heap_caps.h"

static ESP32_C3_LCD_1in47 lcd;
static Arduino_TFT *tft = NULL;
static JPEGDEC jpeg;
static uint16_t *jpgDrawBuf = NULL;
static uint32_t jpgDrawBufPixels = 0;

static uint16_t swap565(uint16_t v)
{
  return (uint16_t)((v << 8) | (v >> 8));
}

static bool ensureJpgDrawBuf(uint32_t pixels)
{
  if (pixels <= jpgDrawBufPixels) {
    return true;
  }

  uint16_t *buf = (uint16_t *)heap_caps_malloc(pixels * sizeof(uint16_t), MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
  if (buf == NULL) {
    return false;
  }

  if (jpgDrawBuf != NULL) {
    heap_caps_free(jpgDrawBuf);
  }

  jpgDrawBuf = buf;
  jpgDrawBufPixels = pixels;
  return true;
}

static bool loadFileToRam(const char *path, uint8_t **data, int32_t *size)
{
  File file;
  uint8_t *buf = NULL;
  int32_t fileSize = 0;
  int32_t pos = 0;

  *data = NULL;
  *size = 0;

  lcd.selectSD();
  file = SD.open(path, FILE_READ);
  if (!file) {
    lcd.selectLCD();
    return false;
  }

  fileSize = (int32_t)file.size();
  if (fileSize <= 0) {
    file.close();
    lcd.selectLCD();
    return false;
  }

  buf = (uint8_t *)heap_caps_malloc((size_t)fileSize, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
  if (buf == NULL) {
    file.close();
    lcd.selectLCD();
    return false;
  }

  while (true) {
    int32_t remain = fileSize - pos;
    int32_t chunk = remain;

    if (remain <= 0) {
      break;
    }

    if (chunk > 4096) {
      chunk = 4096;
    }

    int32_t r = (int32_t)file.read(&buf[pos], (size_t)chunk);
    if (r <= 0) {
      break;
    }
    pos += r;
  }

  file.close();
  lcd.selectLCD();

  if (pos != fileSize) {
    heap_caps_free(buf);
    return false;
  }

  *data = buf;
  *size = fileSize;
  return true;
}

static int jpgDraw(JPEGDRAW *draw)
{
  uint32_t pixels = (uint32_t)draw->iWidth * (uint32_t)draw->iHeight;
  uint32_t i = 0;

  if (!ensureJpgDrawBuf(pixels)) {
    return 0;
  }

  while (i < pixels) {
    jpgDrawBuf[i] = swap565(draw->pPixels[i]);
    ++i;
  }

  tft->draw16bitRGBBitmap(draw->x, draw->y, jpgDrawBuf, draw->iWidth, draw->iHeight);
  return 1;
}

static bool findFirstJpg(char *path, size_t size)
{
  File root;
  File file;

  lcd.selectSD();
  root = SD.open("/");
  if (!root || !root.isDirectory()) {
    lcd.selectLCD();
    return false;
  }

  while (true) {
    file = root.openNextFile();
    if (!file) {
      break;
    }
    if (!file.isDirectory()) {
      String name = file.name();
      String low = name;
      low.toLowerCase();
      if (low.endsWith(".jpg") || low.endsWith(".jpeg")) {
        if (name.length() > 0 && name[0] != '/') {
          snprintf(path, size, "/%s", name.c_str());
        } else {
          strlcpy(path, name.c_str(), size);
        }
        file.close();
        root.close();
        lcd.selectLCD();
        return true;
      }
    }
    file.close();
  }

  root.close();
  lcd.selectLCD();
  return false;
}

static bool drawJpg(const char *path)
{
  uint8_t *data = NULL;
  int32_t size = 0;

  if (!loadFileToRam(path, &data, &size)) {
    return false;
  }

  lcd.selectLCD();
  int rc = jpeg.openRAM(data, size, jpgDraw);
  if (!rc) {
    lcd.selectLCD();
    heap_caps_free(data);
    return false;
  }

  jpeg.setPixelType(RGB565_BIG_ENDIAN);
  rc = jpeg.decode(0, 0, 0);
  jpeg.close();
  lcd.selectLCD();
  heap_caps_free(data);
  return rc != 0;
}

void setup(void)
{
  char jpgPath[128] = {0};

  Serial.begin(115200);
  delay(50);
  Serial.println("ESP32_C3_LCD_1in47 SD example");

  if (!lcd.begin()) {
    Serial.printf("lcd.begin failed: %s\r\n", esp_err_to_name(lcd.lastError()));
    while (true) {
      delay(1000);
    }
  }

  tft = static_cast<Arduino_TFT *>(lcd.gfx());
  tft->fillScreen(RGB565_BLACK);

  if (!lcd.beginSD()) {
    Serial.printf("beginSD failed: %s\r\n", esp_err_to_name(lcd.lastError()));
    while (true) {
      delay(1000);
    }
  }

  lcd.selectSD();
  if (!SD.begin(10, SPI, 16000000UL)) {
    lcd.selectLCD();
    Serial.println("SD.begin failed");
    tft->setCursor(8, 8);
    tft->setTextColor(RGB565_RED);
    tft->println("SD.begin failed");
    while (true) {
      delay(1000);
    }
  }
  lcd.selectLCD();

  if (!findFirstJpg(jpgPath, sizeof(jpgPath))) {
    Serial.println("jpg not found");
    tft->setCursor(8, 8);
    tft->setTextColor(RGB565_RED);
    tft->println("jpg not found");
    while (true) {
      delay(1000);
    }
  }

  Serial.printf("draw: %s\r\n", jpgPath);
  tft->fillScreen(RGB565_BLACK);
  drawJpg(jpgPath);
}

void loop(void)
{
  delay(1000);
}
