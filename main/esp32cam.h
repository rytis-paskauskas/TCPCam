/* esp32cam.h --- TCPCam  -*- mode: C; -*- 
 * 
 * Copyright (C) 2022, Rytis Paškauskas <rytis.paskauskas@gmail.com>
 *
 * Author: Rytis Paškauskas
 * URL: https://github.com/rytis-paskauskas/TCPCam
 * Created: 2022-03-03
 * Last modified: 2022-03-03 21:03:31 (CET) +0100
 *
 *
 */
#ifndef esp32cam_h_defined
#define esp32cam_h_defined

#include <esp_err.h>
#include "tcp_server.h"
esp_err_t camera_init(void);
struct tcp_payload* tcp_payload_esp32cam(void);

#endif /* esp32cam_h_defined */
