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
## esp32 ble gatt server 蓝牙gatt服务端
### ble gatt server config gatt服务端配置
```c
struct gatts_profile_inst
{
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
}; // gatt服务端配置profile
```
### ble gatt server init gatt服务端初始化
```c
void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
esp_ble_gatts_register_callback(gatts_profile_event_handler); // 注册gatt事件处理函数
esp_ble_gatts_app_register(PROFILE_APP_IDX); // 注册gatt应用
```
### ble gatt server event handler  gatt服务端事件处理
```c
void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
switch (event)
    {
    // Handle GATT Server events here
    case ESP_GATTS_REG_EVT: // 注册事件
    {
        if (param->reg.status == ESP_GATT_OK)
        {
            gl_profile_tab[param->reg.app_id].gatts_if = gatts_if;
        }
        else
        {
            printf("Reg app failed, app_id %04x, status %d\n",
                   param->reg.app_id,
                   param->reg.status);
            return;
        }

        gl_profile_tab[PROFILE_APP_IDX].service_id.is_primary = true;
        gl_profile_tab[PROFILE_APP_IDX].service_id.id.inst_id = 0x00; //设置service_id
        gl_profile_tab[PROFILE_APP_IDX].service_id.id.uuid.len = ESP_UUID_LEN_128; 
        gl_profile_tab[PROFILE_APP_IDX].service_id.id.uuid.uuid.uuid128[0] = 0x12;
        gl_profile_tab[PROFILE_APP_IDX].service_id.id.uuid.uuid.uuid128[1] = 0x34;
        gl_profile_tab[PROFILE_APP_IDX].service_id.id.uuid.uuid.uuid128[2] = 0x56;
        gl_profile_tab[PROFILE_APP_IDX].service_id.id.uuid.uuid.uuid128[3] = 0x78;
        gl_profile_tab[PROFILE_APP_IDX].service_id.id.uuid.uuid.uuid128[4] = 0x90;
        gl_profile_tab[PROFILE_APP_IDX].service_id.id.uuid.uuid.uuid128[5] = 0xAB;
        gl_profile_tab[PROFILE_APP_IDX].service_id.id.uuid.uuid.uuid128[6] = 0xCD;
        gl_profile_tab[PROFILE_APP_IDX].service_id.id.uuid.uuid.uuid128[7] = 0xEF;
        gl_profile_tab[PROFILE_APP_IDX].service_id.id.uuid.uuid.uuid128[8] = 0x12;
        gl_profile_tab[PROFILE_APP_IDX].service_id.id.uuid.uuid.uuid128[9] = 0x34;
        gl_profile_tab[PROFILE_APP_IDX].service_id.id.uuid.uuid.uuid128[10] = 0x56;
        gl_profile_tab[PROFILE_APP_IDX].service_id.id.uuid.uuid.uuid128[11] = 0x78;
        gl_profile_tab[PROFILE_APP_IDX].service_id.id.uuid.uuid.uuid128[12] = 0x90;
        gl_profile_tab[PROFILE_APP_IDX].service_id.id.uuid.uuid.uuid128[13] = 0xAB;
        gl_profile_tab[PROFILE_APP_IDX].service_id.id.uuid.uuid.uuid128[14] = 0xCD;
        gl_profile_tab[PROFILE_APP_IDX].service_id.id.uuid.uuid.uuid128[15] = 0xEF;

        esp_err_t ret = esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_APP_IDX].service_id, 4); //注意最后一个参数一定要够大，不然创建char会失败

        break;
    }
    case ESP_GATTS_CREATE_EVT: // 创建事件
    {

        gl_profile_tab[PROFILE_APP_IDX].service_handle = param->create.service_handle;               // 获取service_handle
        gl_profile_tab[PROFILE_APP_IDX].char_uuid.len = ESP_UUID_LEN_16;                             // 设置char_uuid
        gl_profile_tab[PROFILE_APP_IDX].char_uuid.uuid.uuid16 = 0xFF03;                              // 设置char_uuid
        esp_err_t ret = esp_ble_gatts_start_service(gl_profile_tab[PROFILE_APP_IDX].service_handle); // 启动service

        esp_bt_uuid_t char_uuid = {
            .len = ESP_UUID_LEN_16,
            .uuid = {
                .uuid16 = 0xFF01,
            },
        }; // 设置char_uuid
        esp_gatt_char_prop_t property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE; // 设置char属性
        esp_attr_control_t control = {
            .auto_rsp = ESP_GATT_AUTO_RSP,
        };
        esp_gatt_perm_t perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE; // 设置权限
        uint8_t char_value[] = "Hello World!";

        esp_attr_value_t attr_value = {
            .attr_max_len = sizeof(char_value),
            .attr_len = sizeof(char_value),
            .attr_value = char_value,
        };
        ret = esp_ble_gatts_add_char(gl_profile_tab[PROFILE_APP_IDX].service_handle,&char_uuid,perm,property,&attr_value,&control); // 添加char

    }
    case ESP_GATTS_START_EVT:
    {
        break;
    }

    case ESP_GATTS_ADD_CHAR_EVT:
    {
        uint16_t length = 0;
        const uint8_t *prf_char;
        gl_profile_tab[PROFILE_APP_IDX].char_handle = param->add_char.attr_handle;
        gl_profile_tab[PROFILE_APP_IDX].descr_uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_APP_IDX].descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG; // 设置char描述符uuid

        esp_err_t ret = esp_ble_gatts_add_char_descr(gl_profile_tab[PROFILE_APP_IDX].service_handle, &gl_profile_tab[PROFILE_APP_IDX].descr_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, NULL, NULL); // 添加char描述符
        break;
    } 
    case ESP_GATTS_ADD_CHAR_DESCR_EVT:
    {
        gl_profile_tab[PROFILE_APP_IDX].descr_handle = param->add_char_descr.attr_handle; // 获取描述符句柄
        break;
    }
    case ESP_GATTS_CONNECT_EVT:
    {
        gl_profile_tab[PROFILE_APP_IDX].conn_id = param->connect.conn_id; // 获取连接id
        break;
    }
    case ESP_GATTS_DISCONNECT_EVT:
    {
        gl_profile_tab[PROFILE_APP_IDX].conn_id = 0; // 断开连接
        esp_ble_gap_start_advertising(&adv_params); // 开始广播

        break;
    }

    default:
        break;
    }

}
```
## esp32 lvgl lvgl图形化界面
### lvgl 安装
```powershell
git clone https://github.com/lvgl/lv_port_esp32.git
```
将此目录下的components文件夹的子文件夹复制到idf的components文件夹下
如果遇到编译不通过，根据编译器的提示更改相应的部分即可
### lvgl 初始化
#### 首先通过menuconfig配置lvgl  
- 注意配置屏幕的大小，spi的针脚等   
- 在components/lvgl_esp32_drivers/lvgl_tft/st7785s.h中配置屏幕大小
（根据具体显示控制器的型号配置）
#### lvgl初始化
```c
lv_init(); // 初始化lvgl
lvgl_driver_init(); // 初始化lvgl驱动
```
创建缓冲区
```c
lv_color_t *buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), (1 << 3));// 创建缓冲区
assert(buf1 != NULL);
memset(buf1, 0, DISP_BUF_SIZE * sizeof(lv_color_t)); // 初始化缓冲区
lv_disp_buf_t disp_buf;
lv_disp_buf_init(&disp_buf, buf1, NULL, DISP_BUF_SIZE); // 初始化disp_buf
```
配置驱动
```c
lv_disp_drv_t disp_drv;
lv_disp_drv_init(&disp_drv);
disp_drv.flush_cb = disp_driver_flush; // 设置刷新函数
disp_drv.buffer = &disp_buf;
lv_disp_drv_register(&disp_drv); // 注册驱动
```
创建屏幕，创建label
```c
lv_obj_t *scr = lv_disp_get_scr_act(NULL); // 获取屏幕
assert(scr != NULL);
lv_obj_t *label = lv_label_create(scr, NULL); // 创建label
lv_label_set_text(label, "Hello World!");
lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0); // 设置label位置
```
定时调用lvgl的tick函数，更新界面
```c
static void lv_tick_task(void *arg)
{
    (void)arg;
    lv_tick_inc(portTICK_RATE_MS);
}
const esp_timer_create_args_t periodic_timer_args = {
    .callback = &lv_tick_task,
    .name = "periodic_gui"};
esp_timer_handle_t periodic_timer;
ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, portTICK_PERIOD_MS * 1000));
```
