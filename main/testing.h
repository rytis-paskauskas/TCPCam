/* testing.h --- TCPCam Symbol’s function definition is void: true-mode-name
 * 
 * Copyright (C) 2022, Rytis Paškauskas <rytis.paskauskas@gmail.com>
 *
 * Author: Rytis Paškauskas
 * URL: https://github.com/rytis-paskauskas/TCPCam
 * Created: 2022-03-03
 * Last modified: 2022-03-03 10:49:17 (CET) +0100
 *
 *
 */
#ifndef testing_h_defined
#define testing_h_defined

#include "tcp_server.h"
struct tcp_payload* tcp_payload_hello(void);
struct tcp_payload* tcp_payload_img(void);


#endif /* testing_h_defined */
