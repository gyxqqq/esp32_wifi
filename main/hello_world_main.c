/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "../build/config/sdkconfig.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_task_wdt.h"
#include "esp_netif.h"
void app_main(void)
{
    printf("Hello world!\n");
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    // config
    esp_netif_t *sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (sta_netif != NULL)
    {
        esp_netif_destroy(sta_netif);
    }
    sta_netif = esp_netif_create_default_wifi_sta();

    esp_netif_set_hostname(sta_netif, "esp32");

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK)
    {
        printf("esp_wifi_init failed: %d\n", ret);
        return;
    }
    esp_wifi_set_mode(WIFI_MODE_STA);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "scugyx",
            .password = "gyx200404",
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false,
            },
        },
    };
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_err_t err0 = esp_wifi_start();
    if (err0 == ESP_OK)
    {
        printf("wifi start success\n");
    }
    else
    {
        printf("wifi start failed\n");
        printf("Error: %s\n", esp_err_to_name(err0));
    }

    esp_err_t err = esp_wifi_connect();
    if (err == ESP_OK)
    {
        printf("wifi connect success\n");
    }
    else
    {
        printf("wifi connect failed\n");
        printf("Error: %s\n", esp_err_to_name(err));
    }
    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if (esp_flash_get_size(NULL, &flash_size) != ESP_OK)
    {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    for (int i = 10000; i >= 0; i--)
    {

        vTaskDelay(1000 / portTICK_PERIOD_MS);
        // 查询wifi连接状态
        wifi_ap_record_t ap_info;
        esp_err_t err = esp_wifi_sta_get_ap_info(&ap_info);
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
