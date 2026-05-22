#include <ESP32_C3_LCD_1in47.h>

static ESP32_C3_LCD_1in47 lcd;
static Arduino_GFX *gfx = NULL;

void setup(void)
{
  Serial.begin(115200);
  delay(50);
  Serial.println("Arduino_GFX Hello World example");

  if (!lcd.begin()) {
    Serial.printf("lcd.begin failed: %s\r\n", esp_err_to_name(lcd.lastError()));
    while (true) {
      delay(1000);
    }
  }

  gfx = lcd.gfx();
  gfx->fillScreen(RGB565_BLACK);
  gfx->setCursor(10, 10);
  gfx->setTextColor(RGB565_RED);
  gfx->setTextSize(2);
  gfx->println("Hello World!");
  delay(5000);
}

void loop(void)
{
  gfx->setCursor(random(gfx->width()), random(gfx->height()));
  gfx->setTextColor(random(0xffff), random(0xffff));
  gfx->setTextSize(random(6), random(6), random(2));
  gfx->println("Hello World!");
  delay(1000);
}
