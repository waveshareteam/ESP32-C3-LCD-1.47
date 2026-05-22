#include "LVGL_Example.h"
#include "freertos/task.h"

static lv_disp_draw_buf_t disp_buf;
static lv_color_t disp_buf1[EXAMPLE_LCD_H_RES * 40];
static lv_color_t disp_buf2[EXAMPLE_LCD_H_RES * 40];

static void lvgl_benchmark_task(void *arg)
{
    (void)arg;
    lv_demo_benchmark_set_max_speed(true);
    lv_demo_benchmark();

    while (true) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    // switch_bus_to_lcd();
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
    static esp_timer_handle_t tick_timer;

    lv_init();
    lv_disp_draw_buf_init(&disp_buf, disp_buf1, disp_buf2, EXAMPLE_LCD_H_RES * 40);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = EXAMPLE_LCD_H_RES;
    disp_drv.ver_res = EXAMPLE_LCD_V_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    if (tick_timer != NULL) {
        return;
    }

    const esp_timer_create_args_t tick_args = {
        .callback = lvgl_tick_cb,
        .name = "lvgl_tick",
    };
    ESP_ERROR_CHECK(esp_timer_create(&tick_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, 2000));
}

void LVGL_Benchmark_Example(void)
{
    ESP_ERROR_CHECK(IO_EXTENSION_Init());
    ESP_ERROR_CHECK(LCD_Init());
    lvgl_display_init();
    xTaskCreate(lvgl_benchmark_task, "lvgl_bench", 6144, NULL, 5, NULL);
}
