/* esp32cam.c --- TCPCam  -*- mode: C; -*- 
 * 
 * Copyright (C) 2022, Rytis Paškauskas <rytis.paskauskas@gmail.com>
 *
 * Author: Rytis Paškauskas
 * URL: https://github.com/rytis-paskauskas/TCPCam
 * Created: 2022-03-03
 * Last modified: 2022-03-04 09:09:05 (CET) +0100
 *
 *
 */
#include <esp_err.h>
#include <esp_log.h>
#include <esp_camera.h>
#include "tcp_server.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CAM_PIN_PWDN 32
#define CAM_PIN_RESET -1 //software reset will be performed
#define CAM_PIN_XCLK 0
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 21
#define CAM_PIN_D2 19
#define CAM_PIN_D1 18
#define CAM_PIN_D0 5
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22

static const char* TAG = "ESP32 CAM";

esp_err_t camera_init()
{
    static camera_config_t camera_config = {
	.pin_pwdn  = CAM_PIN_PWDN,
	.pin_reset = CAM_PIN_RESET,
	.pin_xclk = CAM_PIN_XCLK,
	.pin_sscb_sda = CAM_PIN_SIOD,
	.pin_sscb_scl = CAM_PIN_SIOC,	
	.pin_d7 = CAM_PIN_D7,
	.pin_d6 = CAM_PIN_D6,
	.pin_d5 = CAM_PIN_D5,
	.pin_d4 = CAM_PIN_D4,
	.pin_d3 = CAM_PIN_D3,
	.pin_d2 = CAM_PIN_D2,
	.pin_d1 = CAM_PIN_D1,
	.pin_d0 = CAM_PIN_D0,
	.pin_vsync = CAM_PIN_VSYNC,
	.pin_href = CAM_PIN_HREF,
	.pin_pclk = CAM_PIN_PCLK,	
	.xclk_freq_hz = 20000000, //EXPERIMENTAL: Set to 16MHz on ESP32-S2 or ESP32-S3 to enable EDMA mode
	.ledc_timer = LEDC_TIMER_0,
	.ledc_channel = LEDC_CHANNEL_0,	
	.pixel_format = PIXFORMAT_JPEG,
#if defined(CONFIG_CAMERA_IMGRES_240X240)
	.frame_size = FRAMESIZE_240X240,
#elif defined(CONFIG_CAMERA_IMGRES_QVGA)
	.frame_size = FRAMESIZE_QVGA,
#elif defined(CONFIG_CAMERA_IMGRES_CIF)
	.frame_size = FRAMESIZE_CIF,
#elif defined(CONFIG_CAMERA_IMGRES_HVGA)
	.frame_size = FRAMESIZE_HVGA,
#elif defined(CONFIG_CAMERA_IMGRES_VGA)
	.frame_size =  FRAMESIZE_VGA,
#elif defined(CONFIG_CAMERA_IMGRES_SVGA)
	.frame_size =  FRAMESIZE_SVGA,
#elif defined(CONFIG_CAMERA_IMGRES_XGA)
	.frame_size =  FRAMESIZE_XGA,
#elif defined(CONFIG_CAMERA_IMGRES_HD)
	.frame_size =  FRAMESIZE_HD,
#elif defined(CONFIG_CAMERA_IMGRES_SXGA)
	.frame_size =  FRAMESIZE_SXGA,
#elif defined(CONFIG_CAMERA_IMGRES_UXGA)
	.frame_size =  FRAMESIZE_UXGA,
/* #else */
/* #error "Camera config frame_size: no valid definition" */
#endif
	.jpeg_quality = 10,
	.fb_count = 1,
	.grab_mode = CAMERA_GRAB_WHEN_EMPTY
    };

    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }
    return ESP_OK;
}

/* The camera must be initialized  */
struct tcp_payload* tcp_payload_esp32cam(void)
{
    struct tcp_payload* t=NULL;
#ifdef CONFIG_BUILD_ESP32CAM
    camera_fb_t *fb = esp_camera_fb_get();
    if (fb==NULL)
    {
	ESP_LOGE(TAG, "Camera capture failed");
    } else {
	if(fb->format != PIXFORMAT_JPEG)
	{
            if(!frame2jpg(fb, 80,(uint8_t**)&(t->buf), &(t->len)))
	    {
                ESP_LOGE(TAG, "JPEG compression failed");
            }
	    esp_camera_fb_return(fb);
	} else {
	    t->len = fb->len;
	    t->buf = (char*)fb->buf;
	}
    }
#endif // CONFIG_BUILD_ESP32CAM
    return t;
}

#ifdef __cplusplus
}
#endif
