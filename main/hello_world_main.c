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
#include "esp_flash.h"
#include "../build/config/sdkconfig.h"
#include "nvs_flash.h"

void app_main(void)
{
    printf("Hello world!\n");
    nvs_flash_init();
    esp_err_t ret0 = nvs_flash_init();
    if (ret0 == ESP_ERR_NVS_NO_FREE_PAGES || ret0 == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // 如果NVS分区被占满，或者NVS库版本更高，需要执行全擦除
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret0 = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret0);
    esp_netif_init();


    while (1)
    {

        vTaskDelay(60000 / portTICK_PERIOD_MS);
        
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
