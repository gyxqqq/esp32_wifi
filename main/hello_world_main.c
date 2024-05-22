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
#include "esp_http_client.h"
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        printf("HTTP_EVENT_ERROR\n");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        printf("HTTP_EVENT_ON_CONNECTED\n");
        break;
    case HTTP_EVENT_HEADER_SENT:
        printf("HTTP_EVENT_HEADER_SENT\n");
        break;
    case HTTP_EVENT_ON_HEADER:
        printf("HTTP_EVENT_ON_HEADER, key=%s, value=%s\n", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        printf("HTTP_EVENT_ON_DATA, len=%d\n", evt->data_len);
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            printf("%.*s", evt->data_len, (char *)evt->data);
        }
        break;
    case HTTP_EVENT_ON_FINISH:
        printf("HTTP_EVENT_ON_FINISH\n");
        break;
    case HTTP_EVENT_DISCONNECTED:
        printf("HTTP_EVENT_DISCONNECTED\n");
        break;
    case HTTP_EVENT_REDIRECT:
        printf("HTTP_EVENT_REDIRECT\n");
        break;
    default:
        printf("HTTP_EVENT_UNKNOWN\n");
        break;
    }
    return ESP_OK;
}
void app_main(void)
{
    printf("Hello world!\n");
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    // config

    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();

    esp_netif_set_hostname(sta_netif, "esp32");
    esp_netif_dns_info_t dns_info;
    IP4_ADDR(&dns_info.ip.u_addr.ip4, 8, 8, 8, 8); // Google 的公共 DNS 服务器 8.8.8.8
    dns_info.ip.type = IPADDR_TYPE_V4;
    esp_netif_set_dns_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), ESP_NETIF_DNS_MAIN, &dns_info);
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
    // creat http client
    esp_http_client_config_t config = {
        .url = "http://www.baidu.com",
        .event_handler = _http_event_handler, // 设置http事件处理函数
    };
    esp_http_client_handle_t client = esp_http_client_init(&config); // 初始化http客户端
    esp_err_t err1 = esp_http_client_perform(client);                // 执行http请求
    if (err == ESP_OK)
    {
        printf("HTTP GET Status = %d, content_length = %lld\n",
               esp_http_client_get_status_code(client),
               esp_http_client_get_content_length(client));
    }
    else
    {
        printf("HTTP GET request failed: %s\n", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);

    for (int i = 10000; i >= 0; i--)
    {

        vTaskDelay(60000 / portTICK_PERIOD_MS);
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
