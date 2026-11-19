#include <ESP32_C3_LCD_1in47.h>

static ESP32_C3_LCD_1in47 lcd;
static Arduino_GFX *gfx = NULL;

static uint8_t backlight = 50;
static bool increase = true;
static bool pressed = false;
static uint8_t clicks = 0;
static uint32_t clickDeadline = 0;

static void drawBarFrame(int16_t x, int16_t y, int16_t w, int16_t h)
{
  gfx->drawRect(x, y, w, h, RGB565_BLACK);
}

static void drawBarValue(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t value)
{
  int16_t fill = (int16_t)((uint32_t)(w - 2) * value / 100);

  gfx->fillRect(x + 1, y + 1, w - 2, h - 2, RGB565_WHITE);
  if (fill > 0) {
    gfx->fillRect(x + 1, y + 1, fill, h - 2, RGB565_BLACK);
  }
}

static void drawUi(void)
{
  gfx->fillScreen(RGB565_WHITE);
  gfx->setTextColor(RGB565_BLACK, RGB565_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(18, 30);
  gfx->println("BOOT Control");

  gfx->setCursor(18, 90);
  gfx->printf("Brightness %u", backlight);

  drawBarFrame(18, 125, 144, 20);
  drawBarValue(18, 125, 144, 20, backlight);

  gfx->setCursor(18, 175);
  gfx->setTextColor(increase ? RGB565_RED : RGB565_GREEN, RGB565_WHITE);
  gfx->println(increase ? "Mode:Increase" : "Mode:Decrease");

  gfx->setCursor(18, 260);
  gfx->setTextColor(RGB565_BLACK, RGB565_WHITE);
  gfx->println("Double Click     Switch");
}

static void applyBacklight(void)
{
  lcd.setBacklight(backlight);
  drawUi();
}

static void stepBacklight(void)
{
  int value = backlight;

  value += increase ? 10 : -10;
  if (value < 0) {
    value = 0;
  }
  if (value > 100) {
    value = 100;
  }
  backlight = (uint8_t)value;
  applyBacklight();
}

static void pollBootKey(void)
{
  bool nowPressed = lcd.bootPressed();
  uint32_t now = millis();

  if (nowPressed != pressed) {
    pressed = nowPressed;
    if (!pressed) {
      clicks++;
      clickDeadline = now + 250;
    }
  }

  if (clicks == 0 || pressed) {
    return;
  }
  if ((int32_t)(now - clickDeadline) < 0) {
    return;
  }
  if (clicks >= 2) {
    increase = !increase;
    applyBacklight();
  } else {
    stepBacklight();
  }
  clicks = 0;
}

void setup(void)
{
  Serial.begin(115200);
  delay(50);
  Serial.println("ESP32_C3_LCD_1in47 Brightness example");

  if (!lcd.begin(backlight)) {
    Serial.printf("lcd.begin failed: %s\r\n", esp_err_to_name(lcd.lastError()));
    while (true) {
      delay(1000);
    }
  }

  gfx = lcd.gfx();
  applyBacklight();
}

void loop(void)
{
  pollBootKey();
  delay(10);
}
