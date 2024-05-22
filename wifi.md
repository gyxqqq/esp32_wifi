# esp32wifi
## ESP32 WiFi
### 初始化流程
```c
/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_task_wdt.h"
#include "esp_netif.h"
void app_main(void)
{

    nvs_flash_init(); // 初始化nvs
    esp_netif_init(); // 初始化网络接口
    esp_event_loop_create_default(); // 创建默认事件循环
    // config
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta(); // 创建默认wifi sta网络接口
    esp_netif_set_hostname(sta_netif, "esp32");  // 设置主机名
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // wifi初始化配置
    esp_err_t ret = esp_wifi_init(&cfg); // 初始化wifi
    esp_wifi_set_mode(WIFI_MODE_STA); // 设置wifi模式为sta

    wifi_config_t wifi_config = {  // wifi配置
        .sta = {
            .ssid = "scugyx",
            .password = "******",
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,  // 认证模式
            .pmf_cfg = {
                .capable = true,
                .required = false,
            },
        },
    };
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config); // 设置wifi配置
    esp_err_t err0 = esp_wifi_start(); // 启动wifi
    esp_err_t err = esp_wifi_connect(); // 连接wifi
    /* Print chip information */

    for (int i = 10000; i >= 0; i--)
    {

        vTaskDelay(1000 / portTICK_PERIOD_MS);
        // 查询wifi连接状态
        wifi_ap_record_t ap_info;
        esp_err_t err = esp_wifi_sta_get_ap_info(&ap_info); // 获取wifi ap信息
        // 打印wifi连接状态
        if (err == ESP_OK)
        {
            printf("wifi connect success\n");
            printf("SSID: %s\n", ap_info.ssid);
            printf("RSSI: %d\n", ap_info.rssi);
            printf("Primary channel: %d\n", ap_info.primary);
            printf("Secondary channel: %d\n", ap_info.second);
            printf("Authmode: %d\n", ap_info.authmode);
            printf("Pairwise cipher: %d\n", ap_info.pairwise_cipher);
            printf("Group cipher: %d\n", ap_info.group_cipher);
            printf("Antenna: %d\n", ap_info.ant);
            printf("RSSI: %d\n", ap_info.rssi);
            printf("Country code: %s\n", ap_info.country.cc);
            printf("Country code: %d\n", ap_info.country.schan);
            printf("Country code: %d\n", ap_info.country.nchan);
            printf("wifi connect success\n");
        }
        else
        {
            printf("wifi connect failed\n");
            printf("Error: %s\n", esp_err_to_name(err));
            // reconnect
            esp_wifi_connect();
        }
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
```