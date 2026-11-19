#include "SD_Example.h"
#include <errno.h>
#include <stdbool.h>

#define SD_EXAMPLE_MOUNT_POINT "/sdcard"
#define SD_EXAMPLE_PATH_MAX 128
#define SD_EXAMPLE_RETRY_COUNT 3
#define SD_EXAMPLE_RETRY_DELAY_MS 80

static const char *TAG = "sd_example";
static uint16_t *jpg_rgb565_buf;
static size_t jpg_rgb565_buf_size;

static esp_err_t jpg_decode_buffer_init(void)
{
    size_t required_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * sizeof(uint16_t);

    if (jpg_rgb565_buf != NULL) {
        return ESP_OK;
    }

    jpg_rgb565_buf = malloc(required_size);
    if (jpg_rgb565_buf == NULL) {
        return ESP_ERR_NO_MEM;
    }

    jpg_rgb565_buf_size = required_size;
    return ESP_OK;
}

// Convert character to lowercase
static char lower_char(char value)
{
    if (value >= 'A' && value <= 'Z') {
        return (char)(value + ('a' - 'A'));
    }
    return value;
}

// Check if a string ends with a suffix, ignoring case
static bool has_suffix_ignore_case(const char *text, const char *suffix)
{
    size_t text_len = strlen(text);
    size_t suffix_len = strlen(suffix);

    if (text_len < suffix_len) {
        return false;
    }

    text += text_len - suffix_len;
    while (*suffix != '\0') {
        if (lower_char(*text) != *suffix) {
            return false;
        }
        text++;
        suffix++;
    }
    return true;
}

static esp_err_t sd_init_with_retry(void)
{
    esp_err_t ret = ESP_FAIL;
    uint8_t attempt = 0;

    while (attempt < SD_EXAMPLE_RETRY_COUNT) {
        ret = SD_Init();
        if (ret == ESP_OK) {
            return ret;
        }
        ESP_LOGW(TAG, "SD init retry %u/%u, err=%s",
                 (unsigned int)(attempt + 1),
                 (unsigned int)SD_EXAMPLE_RETRY_COUNT,
                 esp_err_to_name(ret));
        vTaskDelay(pdMS_TO_TICKS(SD_EXAMPLE_RETRY_DELAY_MS));
        attempt++;
    }

    return ret;
}

static esp_err_t sd_find_jpg(char *path, size_t path_size)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    size_t mount_len = strlen(SD_EXAMPLE_MOUNT_POINT);
    size_t name_len = 0;
    esp_err_t ret = ESP_ERR_NOT_FOUND;


    switch_bus_to_sd();
    dir = opendir(SD_EXAMPLE_MOUNT_POINT);
    if (dir == NULL) {
        switch_bus_to_lcd();
        return ESP_FAIL;
    }

    while (true) {
        entry = readdir(dir);
        if (entry == NULL) {
            break;
        }
        if (!has_suffix_ignore_case(entry->d_name, ".jpg") &&
            !has_suffix_ignore_case(entry->d_name, ".jpeg")) {
            continue;
        }

        name_len = strnlen(entry->d_name, sizeof(entry->d_name));
        if ((mount_len + 1 + name_len + 1) > path_size) {
            closedir(dir);
            switch_bus_to_lcd();
            return ESP_ERR_INVALID_SIZE;
        }

        memcpy(path, SD_EXAMPLE_MOUNT_POINT, mount_len);
        path[mount_len] = '/';
        memcpy(&path[mount_len + 1], entry->d_name, name_len);
        path[mount_len + 1 + name_len] = '\0';
        ret = ESP_OK;
        break;
    }

    closedir(dir);
    switch_bus_to_lcd();
    return ret;
}

/*
 * @brief  Read a file from SD card
 * @param  path: File path
 * @param  jpg_data: Pointer to store the file data
 * @param  jpg_size: Pointer to store the file size
 * @return esp_err_t: ESP_OK on success, other error codes otherwise
 */
static esp_err_t sd_read_file(const char *path, uint8_t **jpg_data, size_t *jpg_size)
{
    struct stat file_stat = {0};
    FILE *file = NULL;
    size_t read_size = 0;
    esp_err_t ret = ESP_OK;

    *jpg_data = NULL;
    *jpg_size = 0;

    switch_bus_to_sd();

    if (stat(path, &file_stat) != 0) {
        ESP_LOGE(TAG, "stat failed: %s", path);
        ret = ESP_FAIL;
        goto EXIT;
    }

    if (file_stat.st_size <= 0) {
        ret = ESP_ERR_INVALID_SIZE;
        goto EXIT;
    }

    *jpg_data = malloc((size_t)file_stat.st_size);
    if (*jpg_data == NULL) {
        ret = ESP_ERR_NO_MEM;
        goto EXIT;
    }

    file = fopen(path, "rb");
    if (file == NULL) {
        ESP_LOGE(TAG, "open file failed: %s", path);
        ret = ESP_FAIL;
        goto EXIT;
    }

    read_size = fread(*jpg_data, 1, (size_t)file_stat.st_size, file);
    if (read_size != (size_t)file_stat.st_size) {
        ESP_LOGE(TAG, "read file failed: %s", path);
        ret = ESP_FAIL;
        goto EXIT;
    }

    *jpg_size = read_size;
    ESP_LOGI(TAG, "jpg loaded: %s, %u bytes", path, (unsigned int)*jpg_size);

EXIT:
    if (file != NULL) {
        fclose(file);
    }

    switch_bus_to_lcd();

    if (ret == ESP_OK) {
        return ret;
    }

    free(*jpg_data);
    *jpg_data = NULL;
    *jpg_size = 0;
    return ret;
}

static esp_err_t sd_read_file_retry(const char *path, uint8_t **jpg_data, size_t *jpg_size)
{
    esp_err_t ret = ESP_FAIL;
    uint8_t attempt = 0;

    while (attempt < SD_EXAMPLE_RETRY_COUNT) {
        ret = sd_read_file(path, jpg_data, jpg_size);
        if (ret == ESP_OK) {
            return ret;
        }
        ESP_LOGW(TAG, "read jpg retry %u/%u, err=%s",
                 (unsigned int)(attempt + 1),
                 (unsigned int)SD_EXAMPLE_RETRY_COUNT,
                 esp_err_to_name(ret));
        vTaskDelay(pdMS_TO_TICKS(SD_EXAMPLE_RETRY_DELAY_MS));
        attempt++;
    }

    return ret;
}

/*
 * @brief  Draw a JPG image on the LCD
 * @param  jpg_data: JPG image data
 * @param  jpg_size: JPG image size
 * @return esp_err_t: ESP_OK on success, other error codes otherwise
 */
static esp_err_t lcd_draw_jpg(const uint8_t *jpg_data, size_t jpg_size)
{
    esp_jpeg_image_cfg_t jpeg_cfg = {
        .indata = (uint8_t *)jpg_data,
        .indata_size = jpg_size,
        .outbuf = (uint8_t *)jpg_rgb565_buf,
        .outbuf_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * sizeof(uint16_t),
        .out_format = JPEG_IMAGE_FORMAT_RGB565,
        .out_scale = JPEG_IMAGE_SCALE_0,
        .flags = {
            .swap_color_bytes = 0,
        },
    };
    esp_jpeg_image_output_t outimg = {0};

    ESP_RETURN_ON_ERROR(esp_jpeg_decode(&jpeg_cfg, &outimg), TAG, "jpg decode failed");
    ESP_RETURN_ON_FALSE(outimg.width == EXAMPLE_LCD_H_RES, ESP_ERR_INVALID_SIZE, TAG, "jpg width mismatch");
    ESP_RETURN_ON_FALSE(outimg.height == EXAMPLE_LCD_V_RES, ESP_ERR_INVALID_SIZE, TAG, "jpg height mismatch");

    switch_bus_to_lcd();
    return esp_lcd_panel_draw_bitmap(panel_handle,
                                     0,
                                     0,
                                     EXAMPLE_LCD_H_RES,
                                     EXAMPLE_LCD_V_RES,
                                     jpg_rgb565_buf);
}

void SD_Example(void)
{
    esp_err_t ret = ESP_OK;
    char jpg_path[SD_EXAMPLE_PATH_MAX] = {0};
    uint8_t *jpg_data = NULL;
    size_t jpg_size = 0;

    ESP_GOTO_ON_ERROR(IO_EXTENSION_Init(), EXIT, TAG, "IO init failed");
    ESP_GOTO_ON_ERROR(LCD_Init(), EXIT, TAG, "LCD init failed");
    ESP_GOTO_ON_ERROR(jpg_decode_buffer_init(), EXIT, TAG, "alloc global jpg buffer failed");
    ESP_GOTO_ON_ERROR(sd_init_with_retry(), EXIT, TAG, "SD init failed");
    ESP_GOTO_ON_ERROR(sd_find_jpg(jpg_path, sizeof(jpg_path)), EXIT, TAG, "jpg not found");
    ESP_GOTO_ON_ERROR(sd_read_file_retry(jpg_path, &jpg_data, &jpg_size), EXIT, TAG, "read jpg failed");
    ESP_GOTO_ON_ERROR(lcd_draw_jpg(jpg_data, jpg_size), EXIT, TAG, "draw jpg failed");

EXIT:
    free(jpg_data);
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
