/* tcp_server.h --- TCPCam  -*- mode: C; -*- 
 * 
 * Copyright (C) 2022, Rytis Paškauskas <rytis.paskauskas@gmail.com>
 *
 * Author: Rytis Paškauskas
 * URL: https://github.com/rytis-paskauskas/TCPCam
 * Created: 2022-02-25
 * Last modified: 2022-03-03 13:52:07 (CET) +0100
 *
 *
 */
#ifndef tcp_server_h_defined
#define tcp_server_h_defined
#include <stdint.h>		/* uint32_t */

struct tcp_payload {
    uint32_t len;		/* Size of payload in bytes */
    char *buf;			/* The payload */
};

/* Signature for custom payload generators. */
typedef struct tcp_payload* (*tcp_payload_generator_fn)(void); 


/* Helper functions to allocate / deallocate payload */
void                tcp_payload_free(struct tcp_payload* p);
struct tcp_payload* tcp_payload_alloc(uint32_t len); /* length in bytes */
struct tcp_payload* tcp_payload_string(const char* message);

/* The main driver. It accepts a tcp_payload_generator_fn argument  */
void tcp_server_start(void *generator_fn);

#endif /* tcp_server_h_defined */
