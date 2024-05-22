# esp32wifi
基于ESP-IDF5.1.0
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
## esp32 http client http客户端
### handle http client
```c
esp_err_t _http_event_handler(esp_http_client_event_t *evt) // http事件处理
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR: // 错误事件
        printf("HTTP_EVENT_ERROR\n");
        break;
    case HTTP_EVENT_ON_CONNECTED: // 连接事件
        printf("HTTP_EVENT_ON_CONNECTED\n");
        break;
    case HTTP_EVENT_HEADER_SENT: // 头部发送事件
        printf("HTTP_EVENT_HEADER_SENT\n");
        break;
    case HTTP_EVENT_ON_HEADER: // 头部事件
        printf("HTTP_EVENT_ON_HEADER, key=%s, value=%s\n", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA: // 数据事件
        printf("HTTP_EVENT_ON_DATA, len=%d\n", evt->data_len);
        if (!esp_http_client_is_chunked_response(evt->client)) // 如果不是分块响应
        {
            printf("%.*s", evt->data_len, (char *)evt->data); // 打印数据
        }
        break;
    case HTTP_EVENT_ON_FINISH: // 结束事件
        printf("HTTP_EVENT_ON_FINISH\n");
        break;
    case HTTP_EVENT_DISCONNECTED: // 断开连接事件
        printf("HTTP_EVENT_DISCONNECTED\n");
        break;
    case HTTP_EVENT_REDIRECT: // 重定向事件
        printf("HTTP_EVENT_REDIRECT\n");
        break;
    default:
        printf("HTTP_EVENT_UNKNOWN\n"); // 未知事件
        break;
    }
    return ESP_OK;
}
```
### http client
```c
#include "esp_http_client.h"
esp_http_client_config_t config = {
    .url = "http://www.baidu.com",
    .event_handler = _http_event_handler, // 设置http事件处理函数
};
esp_http_client_handle_t client = esp_http_client_init(&config); // 初始化http客户端
esp_err_t err = esp_http_client_perform(client); // 执行http请求
```
## dns server config 设置dns服务器
在netif中设置dns服务器
```c
esp_netif_dns_info_t dns_info;
IP4_ADDR(&dns_info.ip.u_addr.ip4, 8, 8, 8, 8); // Google 的公共 DNS 服务器 8.8.8.8
dns_info.ip.type = IPADDR_TYPE_V4;
esp_netif_set_dns_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), ESP_NETIF_DNS_MAIN, &dns_info); // 设置dns服务器
```
## mqtt client mqtt客户端
### mqtt client config
```c
const esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.uri = "mqtt://*******",
};
esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
esp_mqtt_client_start(client);
```
### mqtt event handler
```c
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  
    esp_mqtt_event_handle_t event = event_data; // 获取mqtt事件
    esp_mqtt_client_handle_t client = event->client; // 获取mqtt客户端
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) { // 处理mqtt事件
    case MQTT_EVENT_CONNECTED:
        printf("MQTT_EVENT_CONNECTED\n");
        msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0); // 发布消息 
        printf("sent publish successful, msg_id=%d\n", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0); // 订阅消息
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
        printf("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d\n", event->msg_id); // 取消订阅
        break;
    case MQTT_EVENT_PUBLISHED:
        printf("MQTT_EVENT_PUBLISHED, msg_id=%d\n", event->msg_id); // 发布消息
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
```