/* tcp_server.c --- TCPCam  -*- mode: C; -*- 
 * 
 * Copyright (C) 2022, Rytis Paškauskas <rytis.paskauskas@gmail.com>
 *
 * Author: Rytis Paškauskas
 * URL: https://github.com/rytis-paskauskas
 * Created: 2022-02-25
 * Last modified: 2022-03-04 10:51:15 (CET) +0100
 *
 *
 */
#include <string.h>
#include <stdint.h>
#include <sys/param.h>
#include <sys/queue.h>
#include <esp_log.h>
#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
#include "tcp_server.h"


#ifdef __cplusplus
extern "C" {
#endif

/* For debugging and such */
static const char *TAG = "tcp server";


#define STR1(x)  #x
#define STR2(x) STR1(x)
#define PORT                        CONFIG_TCP_PORT
#define KEEPALIVE_IDLE              CONFIG_KEEPALIVE_IDLE
#define KEEPALIVE_INTERVAL          CONFIG_KEEPALIVE_INTERVAL
#define KEEPALIVE_COUNT             CONFIG_KEEPALIVE_COUNT
    
struct list_elt {
    int fd;
    int req;
    TaskHandle_t tsk;
    LIST_ENTRY(list_elt) open_sockets;
};

LIST_HEAD(list_head, list_elt);
static struct list_head sl_head;
static uint8_t sl_size;
static TaskHandle_t  generator_tsk_h;
static TaskHandle_t  dispatcher_tsk_h;
static QueueHandle_t payloadQueue;

static struct list_elt* elt_alloc(int sock)
{
    struct list_elt *me = malloc(sizeof(struct list_elt));
    assert(me!=NULL);
    me->fd = sock;
    return me;
}

static void elt_free(struct list_elt *p)
{
    if(p!=NULL)
    {
	shutdown(p->fd, 0);
	close(p->fd);
	/* printf("elt_free: %u elements remain\n",sl_size); */
	free(p);
    }
}

static void list_elt_add(struct list_elt* p)
{
    LIST_INSERT_HEAD(&sl_head,p,open_sockets);
    sl_size+=1;
    /* printf("elt_alloc: %u element added\n",sl_size); */
    return;
}

static void list_elt_remove(struct list_elt *p)
{
    LIST_REMOVE(p,open_sockets);
    sl_size-=1;
    return;
}

struct tcp_payload* tcp_payload_alloc(uint32_t len)
{
    struct tcp_payload* t = malloc(sizeof (struct tcp_payload));
    assert(t);
    t->len = len;
    t->buf = malloc(t->len);
    return t;
}

struct tcp_payload* tcp_payload_string(const char* message)
{
    struct tcp_payload* t = tcp_payload_alloc( sizeof(char)*strlen(message));
    memcpy(t->buf, message, t->len);
    return t;
}


void tcp_payload_free(struct tcp_payload* p)
{
    if(p!=NULL)
    {
	if(p->buf!=NULL)
	{
	    free(p->buf);
	    p->buf = NULL;
	}
	free(p);
    }
}

/* Note that some clients append junk bytes at the end, such as 0x0d 0x0a (i.e. \r \n)  - We don't need them */
static int chomp_string(char *buf, int len)
{
    char tmp = buf[len-1];
    while( len>0 && (tmp == '\r' || tmp == '\n' || tmp == '\0'))
    {
	printf("\nworking on:(0x%02x) %c\n", tmp, tmp);
	len = len-1;
	tmp = buf[len-1];
    }
    buf[len] = '\0';
    return len;
}

static void socket_task(void* pvParameters)
{
    int msg = 0;
    int rc=0;
    static uint8_t count = 0;
    int len;
    struct list_elt *ptr = (struct list_elt *)pvParameters;
    char rx_buffer[128];				
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    count += 1;
    /* const uint8_t me = count; */

    /* Receive the credentials */
    /* do{ */
    len = recv(ptr->fd, rx_buffer, sizeof rx_buffer, 0);
    if (len < 0) {
	ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
	goto SOCKET_TASK_DONE;
	/* break; */
    } else if (len==0) {
	ESP_LOGW(TAG, "Connection closed");
	goto SOCKET_TASK_DONE;
    } else {
	/* ESP_LOGI(TAG, "Received %d bytes:%s", len, rx_buffer); */
	len = chomp_string(rx_buffer, len); /* , &len */
	/* len = strlen(rx_buffer); */
	/* ESP_LOGI(TAG, "Remain %d bytes:%s", strlen(rx_buffer), rx_buffer); */
	ESP_LOGD(TAG,"comparing:%s(len=%d) against:%s.", rx_buffer, len, STR2(CONFIG_TCP_PASSWORD));
	/* for(int i=0;i!=len;i++) */
	/*     printf("0x%02x%s", (unsigned int) rx_buffer[i],(i==len-1?"\n":" ")); */
	rc=strcmp(rx_buffer,STR2(CONFIG_TCP_PASSWORD));
	if(rc!=0) {
	    ESP_LOGI(TAG, "fail");
	    goto SOCKET_TASK_DONE;
	}	    
	ESP_LOGI(TAG, "pass");
    }
    
    len = recv(ptr->fd, rx_buffer, sizeof rx_buffer, 0);
    if (len < 0) {
	ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
    } else if (len == 0) {
	ESP_LOGW(TAG, "Connection closed");
	goto SOCKET_TASK_DONE;
    } else {
	len = chomp_string(rx_buffer, len);
	rx_buffer[len]='\0';
	msg = atoi(rx_buffer);
    }
    ptr->req = msg-1;

    setsockopt(ptr->fd, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
    setsockopt(ptr->fd, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
    setsockopt(ptr->fd, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
    setsockopt(ptr->fd, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));

    list_elt_add(ptr);
    
    if(eTaskGetState(generator_tsk_h)==eSuspended) {
	/* Clear the image queue? */
	ESP_LOGI(TAG,"%s wake up", __FUNCTION__);
	vTaskResume(generator_tsk_h);
    }
    /* Activate dispatcher */
    if(eTaskGetState(dispatcher_tsk_h)==eSuspended) {
	/* Clear the image queue? */
	ESP_LOGI(TAG,"%s wake up", __FUNCTION__);
	vTaskResume(dispatcher_tsk_h);
    }
    
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
SOCKET_TASK_DONE:
    /* printf("here\n"); */
    ESP_LOGI(TAG,"Goodbye from socket %d.\n", ptr->fd);
    elt_free(ptr);
    vTaskDelete(NULL);
}

static void content_dispatcher_task(void*Params)
{
    BaseType_t status;
    int rc=0;
    uint32_t tmp;
    struct list_elt *cur, *nxt;
    struct tcp_payload* p;
    ESP_LOGI(TAG,"%s going to sleep.", __FUNCTION__);
    vTaskSuspend(NULL);
    for(;;)
    {
	ESP_LOGD(TAG, "%s: %u images waiting, %u sockets available\n",__FUNCTION__, uxQueueMessagesWaiting(payloadQueue),sl_size);
	status = xQueueReceive(payloadQueue,&p,portMAX_DELAY);
	if(status==pdFALSE)
	{
	    ESP_LOGE(TAG, "Failed to receive data from queue");
	}
	cur=LIST_FIRST(&sl_head);
	tmp = htonl(p->len);
	while(cur!=NULL)
	{
	    ESP_LOGD(TAG, "%s: contacting socket %d and sending packet with payload(%u bytes)...\n", __FUNCTION__, cur->fd, p->len);
	    nxt = LIST_NEXT(cur,open_sockets);
	    rc=send(cur->fd, (char*)&tmp, sizeof p->len, 0);
	    if(rc<=0)
	    {
		/* connection failure */
		goto CONTENT_DISPATCHER_BAD_SOCKET;
	    } else if(rc!=sizeof p->len)
	    {
		/* failed to send packet */
		ESP_LOGE(TAG, "%s failed to send packet header", __FUNCTION__);
	    } else {
		/* All went well */
		/* assert(rc==sizeof p->len); */
		rc=send(cur->fd, p->buf, p->len, 0);
		if(rc<=0)
		{
		    /* connection failure */
		} else if(rc!=p->len) {
		    goto CONTENT_DISPATCHER_BAD_SOCKET;
		} else {
		    /* all went well */
		    /* assert(rc==p->len); */
		}
	    }
	CONTENT_DISPATCHER_BAD_SOCKET:
	    if(rc<0 || cur->req==0) {
		list_elt_remove(cur);
		xTaskNotifyGive(cur->tsk);
	    } else if(cur->req>0) {
		cur->req -= 1;
	    } /* otherwise cur->req<0 : indefinite iteration */
	    cur = nxt;
	}
	tcp_payload_free(p);
	if(sl_size == 0)
	{
	    vTaskSuspend(generator_tsk_h);
	    vTaskSuspend(NULL);
	}
    }
}

static void tcp_server_task(void *pvParameters)
{
    char addr_str[128];
    int ip_protocol = 0;
    int opt = 1;
    int listen_sock;
    int rc;
    struct sockaddr_storage dest_addr;
    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(PORT);
    ip_protocol = IPPROTO_IP;
    
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, ip_protocol)) < 0)
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
    }
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    rc=listen(listen_sock, 3);
    if (rc!=0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }
    LIST_INIT(&sl_head);
    sl_size=0;
    while (1)
    {
        ESP_LOGI(TAG, "Socket listening");
        struct sockaddr_storage source_addr;
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }
        if (source_addr.ss_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }
	
        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);
	struct list_elt *ptr = elt_alloc(sock);
	/* This task should be activated before the content and dispatcher tasks.  */
	xTaskCreate(socket_task, "SocketFork", 2048, ptr, uxTaskPriorityGet(NULL)+2, &(ptr->tsk));
    }
CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}

static void content_generator_task(void *pvParameters)
{
    const TickType_t next_frame = pdMS_TO_TICKS(CONFIG_STREAM_PAYLOAD_DELAY);
    BaseType_t status;    
    struct tcp_payload *buf;
    tcp_payload_generator_fn get = (tcp_payload_generator_fn)pvParameters;
    payloadQueue = xQueueCreate(CONFIG_PAYLOAD_QUEUE_MAX, sizeof(struct tcp_payload*));
    assert(payloadQueue);
    ESP_LOGI(TAG,"%s going to sleep.", __FUNCTION__);
    vTaskSuspend(NULL);
    ESP_LOGD(TAG,"%s: starting frame queue.", __FUNCTION__);
    for(;;) {
	buf = get();
	status = xQueueSend(payloadQueue,&buf,portMAX_DELAY);
	if(status !=pdPASS) {
	    ESP_LOGD(TAG, "content_generator failed to add image: queue full");
	} else {
	    ESP_LOGD(TAG, "content_generator success! Frame added.");
	}
	vTaskDelay(next_frame);
    }
    vTaskDelete(NULL);
}

void tcp_server_start(void *generator_fn)
{
    UBaseType_t serverPriority = uxTaskPriorityGet(NULL);
    UBaseType_t handlerPriority = serverPriority+1;
    xTaskCreate(tcp_server_task,         "Custom TCP Server", 4096,NULL, serverPriority, NULL);
    xTaskCreate(content_generator_task,  "Content Generator", 8192, generator_fn, handlerPriority, &generator_tsk_h);
    xTaskCreate(content_dispatcher_task, "Content Dispatcher", 8196, NULL, handlerPriority, &dispatcher_tsk_h);
}
    
#ifdef __cplusplus
}
#endif
