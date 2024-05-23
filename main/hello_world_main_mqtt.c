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
#include "esp_netif.h"
#include "lwip/ip_addr.h"
#include "mqtt_client.h"
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        printf("MQTT_EVENT_CONNECTED\n");
        msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        printf("sent publish successful, msg_id=%d\n", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        printf("sent subscribe successful, msg_id=%d\n", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        printf("sent subscribe successful, msg_id=%d\n", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        printf("MQTT_EVENT_DISCONNECTED\n");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        printf("MQTT_EVENT_SUBSCRIBED, msg_id=%d\n", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        printf("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d\n", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        printf("MQTT_EVENT_PUBLISHED, msg_id=%d\n", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        printf("MQTT_EVENT_DATA\n");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);

        break;
    case MQTT_EVENT_ERROR:
        printf("MQTT_EVENT_ERROR\n");
        
        break;
    default:
        printf("Other event id:%ld\n", event_id);
        break;
    }
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

const esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.uri = "mqtt://jzitan.top:1883",
    .credentials.client_id = "esp32",
};

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
    //mqtt connect
    esp_mqtt_client_subscribe(client, "test", 1);
    esp_mqtt_client_publish(client, "test", "hello", 0, 1, 0);

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
