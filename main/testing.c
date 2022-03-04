/* testing.c --- TCPCam Symbol’s function definition is void: true-mode-name
 * 
 * Copyright (C) 2022, Rytis Paškauskas <rytis.paskauskas@gmail.com>
 *
 * Author: Rytis Paškauskas
 * URL: https://github.com/rytis-paskauskas/TCPCam
 * Created: 2022-03-03
 * Last modified: 2022-03-03 13:50:21 (CET) +0100
 *
 *
 */
#include <string.h>
#include "tcp_server.h"

struct tcp_payload* tcp_payload_hello(void)
{
    return tcp_payload_string("Hello World");
}

struct tcp_payload* tcp_payload_img(void)
{
    extern const uint8_t img_start[] asm("_binary_rose_jpeg_start");
    extern const uint8_t img_end[]   asm("_binary_rose_jpeg_end");
    struct tcp_payload* t = tcp_payload_alloc(img_end - img_start);
    memcpy(t->buf, img_start, t->len);
    return t;
}
