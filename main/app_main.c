/* app_main.c --- TCPCam  -*- mode: C; -*- 
 * 
 * Copyright (C) 2022, Rytis Paškauskas <rytis.paskauskas@gmail.com>
 *
 * Author: Rytis Paškauskas
 * URL: https://github.com/rytis-paskauskas
 * Created: 2022-02-25
 * Last modified: 2022-03-04 09:08:18 (CET) +0100
 *
 *
 */
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_event.h>

#include <esp_log.h>
#include <nvs_flash.h>
#include <lwip/sockets.h>
#include "wifi.h"
#include "tcp_server.h"
#include "testing.h"
#include "esp32cam.h"

#ifdef __cplusplus
extern "C" {
#endif

static void event_handler(void* args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    const char* tag = "MAIN HANDLER";
    ESP_LOGI(tag, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    /* Network layer */
    if(base==IP_EVENT)
    {
	switch((ip_event_t)event_id) {
	case IP_EVENT_STA_GOT_IP:
	    ESP_LOGI(tag, "IP_EVENT: got IP");
	    /* Only one of the following should be active */
#ifdef CONFIG_BUILD_TEST_HELLO
	    tcp_server_start(tcp_payload_hello);
#endif // CONFIG_BUILD_TEST_HELLO
#ifdef CONFIG_BUILD_TEST_JPG
	    tcp_server_start(tcp_payload_img);
#endif // CONFIG_TEST_JPG
#ifdef CONFIG_BUILD_ESP32CAM
	    tcp_server_start(tcp_payload_esp32cam);
#endif // CONFIG_BUILD_ESP32CAM
	    break;
	default:
	    ESP_LOGI(tag, "IP_EVENT: unhandled event_id: %d", event_id);
	    break;
	}
    }
}

void app_main(void)
{
#ifdef CONFIG_BUILD_ESP32CAM
    ESP_ERROR_CHECK(camera_init());
#endif // CONFIG_BUILD_ESP32CAM
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL);
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_basic();
}


#ifdef __cplusplus
}
#endif
