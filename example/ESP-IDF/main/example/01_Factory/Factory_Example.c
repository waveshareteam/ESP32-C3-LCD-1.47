#include "Factory_Example.h"

static const char *TAG = "main";

static lv_disp_draw_buf_t disp_buf;
static lv_color_t buf1[EXAMPLE_LCD_H_RES * 100];
static lv_color_t buf2[EXAMPLE_LCD_H_RES * 100];

static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map);
    lv_disp_flush_ready(drv);
}

static void lvgl_tick_cb(void *arg)
{
    (void)arg;
    lv_tick_inc(2);
}

static void lvgl_display_init(void)
{
    lv_init();

    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, EXAMPLE_LCD_H_RES * 100);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = EXAMPLE_LCD_H_RES;
    disp_drv.ver_res = EXAMPLE_LCD_V_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    static esp_timer_handle_t lvgl_tick_timer = NULL;
    if (lvgl_tick_timer == NULL) {
        const esp_timer_create_args_t lvgl_tick_args = {
            .callback = &lvgl_tick_cb,
            .name = "lvgl_tick",
        };
        ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_args, &lvgl_tick_timer));
    }
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, 2000));
}

void Factory_Example(void)
{
    ESP_LOGI(TAG, "board init");
    IO_EXTENSION_Init();
    ESP_ERROR_CHECK(LCD_Init());
    SD_Init();
    QMI8658_Init();
    Wireless_Init();

    ESP_LOGI(TAG, "lvgl init");
    lvgl_display_init();
    Lvgl_Example1();

    while (true) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
        QMI8658_Loop();
    }
}
