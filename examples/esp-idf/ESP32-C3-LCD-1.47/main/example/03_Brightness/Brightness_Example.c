#include "Brightness_Example.h"

#define BRIGHTNESS_BOOT_GPIO GPIO_NUM_9
#define BRIGHTNESS_CLICK_MS 250
#define BRIGHTNESS_STEP 10

static lv_disp_draw_buf_t disp_buf;
static lv_color_t buf1[EXAMPLE_LCD_H_RES * 40];
static lv_color_t buf2[EXAMPLE_LCD_H_RES * 40];
static lv_obj_t *value_label;
static lv_obj_t *mode_label;
static lv_obj_t *bar;
static uint8_t backlight = 50;
static bool increase = true;
static bool pressed_state;
static uint8_t clicks;
static uint32_t click_deadline;

static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    switch_bus_to_lcd();
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
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, EXAMPLE_LCD_H_RES * 40);

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

static void boot_key_init(void)
{
    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << BRIGHTNESS_BOOT_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&cfg));
}

static void backlight_apply(void)
{
    BK_Light(backlight);
    lv_bar_set_value(bar, backlight, LV_ANIM_OFF);
    lv_label_set_text_fmt(value_label, "Brightness %u%%", backlight);
    lv_label_set_text(mode_label, increase ? "Mode: Increase" : "Mode: Decrease");
    lv_obj_set_style_text_color(mode_label,
                               increase ? lv_palette_main(LV_PALETTE_RED) : lv_palette_main(LV_PALETTE_GREEN),
                               0);
}

static void backlight_step(void)
{
    int value = backlight;
    value += increase ? BRIGHTNESS_STEP : -BRIGHTNESS_STEP;
    if (value < 0) {
        value = 0;
    }
    if (value > 100) {
        value = 100;
    }
    backlight = value;
    backlight_apply();
}

static void ui_init(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
    lv_obj_set_style_text_color(scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "BOOT Control");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 24);

    value_label = lv_label_create(scr);
    lv_obj_align(value_label, LV_ALIGN_CENTER, 0, -24);

    bar = lv_bar_create(scr);
    lv_obj_set_size(bar, 140, 18);
    lv_obj_align(bar, LV_ALIGN_CENTER, 0, 16);
    lv_bar_set_range(bar, 0, 100);
    lv_obj_set_style_bg_color(bar, lv_palette_lighten(LV_PALETTE_GREY, 2), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, lv_color_black(), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_INDICATOR);

    mode_label = lv_label_create(scr);
    lv_obj_align(mode_label, LV_ALIGN_CENTER, 0, 52);
    
    lv_obj_t *tip = lv_label_create(scr);
    lv_label_set_text(tip, "Double Click Switch");
    lv_obj_align(tip, LV_ALIGN_BOTTOM_MID, 0, -28);

    backlight_apply();
}

static void boot_key_poll(void)
{
    bool pressed = gpio_get_level(BRIGHTNESS_BOOT_GPIO) == 0;
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;

    if (pressed != pressed_state) {
        pressed_state = pressed;
        if (pressed == false) {
            clicks++;
            click_deadline = now + BRIGHTNESS_CLICK_MS;
        }
    }

    if (clicks == 0) {
        return;
    }
    if (pressed) {
        return;
    }
    if ((int32_t)(now - click_deadline) < 0) {
        return;
    }
    if (clicks >= 2) {
        increase = !increase;
        backlight_apply();
    }
    if (clicks == 1) {
        backlight_step();
    }
    clicks = 0;
}

void Brightness_Example(void)
{
    ESP_ERROR_CHECK(IO_EXTENSION_Init());
    ESP_ERROR_CHECK(LCD_Init());
    lvgl_display_init();
    boot_key_init();
    ui_init();

    while (true) {
        boot_key_poll();
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
