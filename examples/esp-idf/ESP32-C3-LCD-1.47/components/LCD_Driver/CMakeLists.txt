idf_component_register(
    SRCS
        "ST7789.c"
        "Vernon_ST7789T/Vernon_ST7789T.c"
    INCLUDE_DIRS
        "."
    REQUIRES
        driver
        SD_SPI
        esp_lcd
        io_extension
        lvgl__lvgl
)

