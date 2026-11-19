#include <ESP32_C3_LCD_1in47.h>
#include <esp_timer.h>
#include <lvgl.h>
#include <demos/benchmark/lv_demo_benchmark.h>
#include "esp_heap_caps.h"

static ESP32_C3_LCD_1in47 lcd;
static Arduino_GFX *gfx = NULL;

static lv_disp_draw_buf_t drawBuf;
static lv_disp_drv_t dispDrv;
static lv_color_t *buf1 = NULL;
static lv_color_t *buf2 = NULL;
static esp_timer_handle_t lvTickTimer = NULL;

static void lvglFlush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *colorMap)
{
  uint32_t w = (uint32_t)(area->x2 - area->x1 + 1);
  uint32_t h = (uint32_t)(area->y2 - area->y1 + 1);

  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&colorMap->full, w, h);
  lv_disp_flush_ready(drv);
}

static void lvglTick(void *arg)
{
  (void)arg;
  lv_tick_inc(2);
}

static bool initLvgl(void)
{
  uint32_t bufPixels = (uint32_t)gfx->width() * 120;
  size_t bufBytes = bufPixels * sizeof(lv_color_t);

  lv_init();

  buf1 = (lv_color_t *)heap_caps_malloc(bufBytes, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
  buf2 = (lv_color_t *)heap_caps_malloc(bufBytes, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
  if (buf1 == NULL || buf2 == NULL) {
    return false;
  }

  lv_disp_draw_buf_init(&drawBuf, buf1, buf2, bufPixels);
  lv_disp_drv_init(&dispDrv);
  dispDrv.hor_res = gfx->width();
  dispDrv.ver_res = gfx->height();
  dispDrv.flush_cb = lvglFlush;
  dispDrv.draw_buf = &drawBuf;
  lv_disp_drv_register(&dispDrv);

  if (lvTickTimer == NULL) {
    esp_timer_create_args_t tickArgs = {};
    tickArgs.callback = lvglTick;
    tickArgs.name = "lvgl_tick";

    if (esp_timer_create(&tickArgs, &lvTickTimer) != ESP_OK) {
      return false;
    }
    if (esp_timer_start_periodic(lvTickTimer, 2000) != ESP_OK) {
      return false;
    }
  }

  lv_demo_benchmark_set_max_speed(true);
  lv_demo_benchmark();
  return true;
}

void setup(void)
{
  Serial.begin(115200);
  delay(50);
  Serial.println("ESP32_C3_LCD_1in47 LVGL example");

  if (!lcd.begin()) {
    Serial.printf("lcd.begin failed: %s\r\n", esp_err_to_name(lcd.lastError()));
    while (true) {
      delay(1000);
    }
  }

  gfx = lcd.gfx();
  gfx->fillScreen(RGB565_BLACK);

  if (!initLvgl()) {
    Serial.println("LVGL init failed");
    gfx->setCursor(8, 8);
    gfx->setTextColor(RGB565_RED);
    gfx->println("LVGL init failed");
    while (true) {
      delay(1000);
    }
  }
}

void loop(void)
{
  lv_timer_handler();
}
