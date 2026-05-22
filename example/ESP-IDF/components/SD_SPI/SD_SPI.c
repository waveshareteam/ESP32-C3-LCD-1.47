#include "SD_SPI.h"
#include "ST7789.h"
#include "driver/sdspi_host.h"
#include "esp_err.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXAMPLE_MAX_CHAR_SIZE 64
#define MOUNT_POINT "/sdcard"
#define SD_SPI_FREQ_KHZ 100000

static const char *SD_TAG = "SD";

uint32_t Flash_Size = 0;
uint32_t SDCard_Size = 0;

esp_err_t example_write_file(const char *path, char *data) {
  ESP_LOGI(SD_TAG, "Opening file %s", path);
  switch_bus_to_sd();
  FILE *f = fopen(path, "w");
  if (f == NULL) {
    ESP_LOGE(SD_TAG, "Failed to open file for writing");
    switch_bus_to_lcd();
    return ESP_FAIL;
  }
  fprintf(f, data);
  fclose(f);
  switch_bus_to_lcd();
  ESP_LOGI(SD_TAG, "File written");

  return ESP_OK;
}

esp_err_t example_read_file(const char *path) {
  ESP_LOGI(SD_TAG, "Reading file %s", path);
  switch_bus_to_sd();
  FILE *f = fopen(path, "r");
  if (f == NULL) {
    ESP_LOGE(SD_TAG, "Failed to open file for reading");
    switch_bus_to_lcd();
    return ESP_FAIL;
  }
  char line[EXAMPLE_MAX_CHAR_SIZE];
  fgets(line, sizeof(line), f);
  fclose(f);

  // strip newline
  char *pos = strchr(line, '\n');
  if (pos) {
    *pos = '\0';
  }
  switch_bus_to_lcd();
  ESP_LOGI(SD_TAG, "Read from file: '%s'", line);

  return ESP_OK;
}

void Flash_Searching(void) {
  if (esp_flash_get_physical_size(NULL, &Flash_Size) == ESP_OK) {
    Flash_Size = Flash_Size / (uint32_t)(1024 * 1024);
    printf("Flash size: %ld MB\n", Flash_Size);
  } else {
    printf("Get flash size failed\n");
  }
}

esp_err_t SD_Init(void) {
  esp_err_t ret = ESP_OK;
  sdmmc_card_t *card = NULL;
  Flash_Searching();
  switch_bus_to_sd();
  // Options for mounting the filesystem.
  // If format_if_mount_failed is set to true, SD card will be partitioned and
  // formatted in case when mounting fails.  false true
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false,
      .max_files = 5,
      .allocation_unit_size = 16 * 1024
  };
  const char mount_point[] = MOUNT_POINT;
  ESP_LOGI(SD_TAG, "Initializing SD card");

  // Use settings defined above to initialize SD card and mount FAT filesystem.
  // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
  // Please check its source code and implement error recovery when developing
  // production applications.
  ESP_LOGI(SD_TAG, "Using SPI peripheral");

  sdmmc_host_t host = SDSPI_HOST_DEFAULT();
  host.slot = SPI2_HOST;
  host.max_freq_khz = SD_SPI_FREQ_KHZ;

  sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot_config.host_id = SPI2_HOST;
  slot_config.gpio_cs = -1;

  ESP_LOGI(SD_TAG, "Mounting filesystem");
  ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config,
                                &card);

  if (ret != ESP_OK) {
    goto EXIT;
  }

  ESP_LOGI(SD_TAG, "Filesystem mounted");

  // Card has been initialized, print its properties
  sdmmc_card_print_info(stdout, card);
  SDCard_Size =
      ((uint64_t)card->csd.capacity) * card->csd.sector_size / (1024 * 1024);
EXIT:
  switch_bus_to_lcd();
  vTaskDelay(pdMS_TO_TICKS(50));
  BK_Light(100);
  if (ret != ESP_OK) {
    ESP_LOGE(SD_TAG, "SD init failed: %s", esp_err_to_name(ret));
  }
  return ret;
}
