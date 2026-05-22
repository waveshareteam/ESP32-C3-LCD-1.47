#ifndef __SD_MMC_H__
#define __SD_MMC_H__
#pragma once

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"

#define CONFIG_EXAMPLE_PIN_CLK  7
#define CONFIG_EXAMPLE_PIN_CMD  5
#define CONFIG_EXAMPLE_PIN_D0   6
#define CONFIG_EXAMPLE_PIN_D1   -1
#define CONFIG_EXAMPLE_PIN_D2   -1
#define CONFIG_EXAMPLE_PIN_D3   -1

esp_err_t example_write_file(const char *path, char *data);
esp_err_t example_read_file(const char *path);

extern uint32_t SDCard_Size;
extern uint32_t Flash_Size;

esp_err_t SD_Init(void);
void Flash_Searching(void);

#endif
