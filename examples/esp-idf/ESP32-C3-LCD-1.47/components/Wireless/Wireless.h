#pragma once

#include <stdbool.h>
#include <stdint.h>

extern bool wifi_sta_connected;
extern uint8_t wifi_ap_sta_count;

void Wireless_Init(void);
void WIFI_Init(void *arg);
