#include "ST7789.h"
#include <stdlib.h>
#include "esp_rom_sys.h"
#include "freertos/idf_additions.h"
#include "io_extension.h"
#include "Vernon_ST7789T/Vernon_ST7789T.h"

static const char *TAG_LCD = "WS_LCD";

esp_lcd_panel_handle_t panel_handle = NULL;

typedef enum {
    BUS_OWNER_NONE = 0,
    BUS_OWNER_LCD,
    BUS_OWNER_SD,
} bus_owner_t;

static bus_owner_t bus_owner = BUS_OWNER_NONE;

static void lcd_spi_select(bus_owner_t owner)
{
    if (bus_owner == owner) {
        return;
    }
    if (owner == BUS_OWNER_LCD) {
        IO_EXTENSION_Output(IO_EXTENSION_SD_CS, 1);
        IO_EXTENSION_Output(IO_EXTENSION_LCD_CS, 0);
        bus_owner = owner;
        return;
    }
    IO_EXTENSION_Output(IO_EXTENSION_LCD_CS, 1);
    IO_EXTENSION_Output(IO_EXTENSION_SD_CS, 0);
    bus_owner = owner;
}

void switch_bus_to_sd(void)
{
    vTaskDelay(pdMS_TO_TICKS(10));
    lcd_spi_select(BUS_OWNER_SD);
    vTaskDelay(pdMS_TO_TICKS(10));
}

void switch_bus_to_lcd(void)
{
    vTaskDelay(pdMS_TO_TICKS(10));
    lcd_spi_select(BUS_OWNER_LCD);
    vTaskDelay(pdMS_TO_TICKS(10));
}

esp_err_t spi_bus_init(void)
{
    spi_bus_config_t buscfg = {
        .sclk_io_num = EXAMPLE_PIN_NUM_SCLK,
        .mosi_io_num = EXAMPLE_PIN_NUM_MOSI,
        .miso_io_num = EXAMPLE_PIN_NUM_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * sizeof(uint16_t),
    };
    esp_err_t ret = spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret == ESP_ERR_INVALID_STATE) {
        return ESP_OK;
    }
    return ret;
}

static esp_err_t lcd_prepare_exio_lines(void)
{
    esp_err_t ret = ESP_OK;
    switch_bus_to_lcd();

    ret = IO_EXTENSION_Output(IO_EXTENSION_LCD_RST, 1);
    if (ret != ESP_OK) {
        return ret;
    }
    vTaskDelay(pdMS_TO_TICKS(20));

    ret = IO_EXTENSION_Output(IO_EXTENSION_LCD_RST, 0);
    if (ret != ESP_OK) {
        return ret;
    }
    vTaskDelay(pdMS_TO_TICKS(20));

    ret = IO_EXTENSION_Output(IO_EXTENSION_LCD_RST, 1);
    if (ret != ESP_OK) {
        return ret;
    }
    vTaskDelay(pdMS_TO_TICKS(120));

    return ESP_OK;
}

void BK_Light(uint8_t Light)
{   
    if (Light > 100) {
        Light = 100;
    }
    switch_bus_to_lcd();
    ESP_ERROR_CHECK(IO_EXTENSION_Pwm_Output(Light));
}
// end Backlight program

esp_err_t LCD_Init(void){
    switch_bus_to_lcd();
    ESP_ERROR_CHECK(lcd_prepare_exio_lines());
    ESP_LOGI(TAG_LCD, "Initialize SPI bus");
    esp_err_t ret = spi_bus_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_LCD, "spi_bus_init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG_LCD, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = EXAMPLE_PIN_NUM_LCD_DC,
        .cs_gpio_num = -1,
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,
        .lcd_param_bits = EXAMPLE_LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 4,
        .on_color_trans_done = NULL,
        .user_ctx = NULL,
    };
    ESP_LOGI(TAG_LCD, "SPI pclk_hz=%u", (unsigned int)io_config.pclk_hz);
    // Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    esp_lcd_panel_dev_st7789t_config_t panel_config = {
        .reset_gpio_num = -1,
        .rgb_endian = LCD_RGB_ENDIAN_BGR,
        .bits_per_pixel = 16,
    };

    ESP_LOGI(TAG_LCD, "Install ST7789T panel driver");
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789t(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    vTaskDelay(pdMS_TO_TICKS(20));
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, LCD_X_GAP, LCD_Y_GAP));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false));
    // user can flush pre-defined pattern to the screen before we turn on the screen or backlight
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    ESP_LOGI(TAG_LCD, "Turn on LCD backlight");
    vTaskDelay(pdMS_TO_TICKS(20));
    BK_Light(100);
    return ESP_OK;
}
