#include "Wireless.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"

void Wireless_Init(void)
{
    // Initialize NVS.
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    // WiFi
    xTaskCreatePinnedToCore(
        WIFI_Init, 
        "WIFI task",
        8192, 
        NULL, 
        1, 
        NULL, 
        0);
}

bool wifi_sta_connected = false;
uint8_t wifi_ap_sta_count = 0;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_ap_sta_count++;
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        if(wifi_ap_sta_count > 0) wifi_ap_sta_count--;
    } else if (event_id == WIFI_EVENT_STA_CONNECTED) {
        wifi_sta_connected = true;
    } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_sta_connected = false;
    }
}

void WIFI_Init(void *arg)
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    cfg.static_rx_buf_num = 6;
    cfg.dynamic_rx_buf_num = 8;
    esp_wifi_init(&cfg);

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_register(WIFI_EVENT,
                                        ESP_EVENT_ANY_ID,
                                        &wifi_event_handler,
                                        NULL,
                                        &instance_any_id);

    wifi_config_t ap_config = {
        .ap = {
            .ssid = "ESP32_C3_LCD_1_47",
            .ssid_len = strlen("ESP32_C3_LCD_1_47"),
            .channel = 1,
            .password = "123456789",
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    wifi_config_t sta_config = {
        .sta = {
            .ssid = "WSTEST",
            .password = "waveshare0755",
        },
    };

    esp_wifi_set_mode(WIFI_MODE_APSTA);
    esp_wifi_set_config(WIFI_IF_AP, &ap_config);
    esp_wifi_set_config(WIFI_IF_STA, &sta_config);
    esp_wifi_start();
    esp_wifi_connect(); // Try connecting as STA
    ESP_LOGI("WIFI", "wifi AP+STA start");

    vTaskDelete(NULL);
}

