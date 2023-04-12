#pragma once
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>

#include "libwebsockets.h"
#include "zlog.h"

#define WS_TX_MAX_LEN (10 * 1024)
#define WS_RX_MAX_LEN (10 * 1024)

#define WS_SUB_PROTOCOL_NUM 3
#define MAX_PAYLOAD_SIZE  10 * 1024
#define MSGSZ 1024

#ifdef __cplusplus
extern "C"
{
#endif

// 子协议栈
enum demo_protocols {
	PROTOCOL_TEST_CALLBACK = 0,
	PROTOCOL_LWS_MIRROR,
	DEMO_PROTOCOL_COUNT
};

enum pthread_status {
	PTHREAD_FILISH = 0,
	PTHREAD_RUN,
	PTHREAD_INIT
};

enum user_cache_status {
	USER_NULL = 0,
	USER_RX,
	USER_TX
};

typedef struct _ws_io_cache_t {
    uint8_t rx[WS_RX_MAX_LEN];
    uint8_t tx[WS_TX_MAX_LEN];
} ws_io_cache_t;


// lws 自定义子协议栈
typedef struct _ws_sub_protocol_t
{
	struct lws_client_connect_info *conn_info;	// 客户端连接信息
	struct lws *wsi_multi;		// 协议接口                         // if (wsi == wsi_multi[n])
    unsigned int rl_multi;		// 连接时间 realtime
    const char *prot_name;		// 协议名称
    
// 1) 通过user寄存收发数据	
	void *p_user;				// user ponit
	int user_len;

// 2) 通过lws数据结构, 处理收发数据
	pthread_t pthread_spam;		// 业务线程
	pthread_mutex_t lock_ring;	/* serialize access to the ring buffer */
	struct lws_ring *ring;		/* ringbuffer 保存未发送的数据 */

	int counter;
	volatile char pthread_state;	// 业务线程状态:
	volatile char established;		// ws连接状态
	volatile char user_state;		// p_user收发状态

    /* io buf*/
    // ws_io_cache_t cache;
} ws_sub_protocol_t;


// user vhost ?
typedef struct _ws_info_t
{
	int protocol_num;                   // 协议数量
	ws_sub_protocol_t sub_prot[WS_SUB_PROTOCOL_NUM];
	// ws_sub_protocol_t **sub_prot;
    
	/* demo*/
    struct lws *wsi_dumb;
    unsigned int rl_dumb;
} ws_info_t;



/**
 * 会话上下文对象，结构根据需要自定义
 */
struct session_data {
    int msg_count;
    unsigned char buf[LWS_PRE + MAX_PAYLOAD_SIZE];
    int len;
};

/**
 * 数据收发 测试队列
 */
typedef struct msgbuf {
    long mtype;         // 消息类型
    char mtext[1024];   // 消息内容
    struct session_data ws_data;
} message_buf;

/* demo refer to lws-minimal-ws-client.c */
struct per_vhost_data__minimal {
	struct lws_context *context;
	struct lws_vhost *vhost;
	const struct lws_protocols *protocol;
	pthread_t pthread_spam[2];

	lws_sorted_usec_list_t sul;

	pthread_mutex_t lock_ring; /* serialize access to the ring buffer */
	struct lws_ring *ring; /* ringbuffer holding unsent messages */
	uint32_t tail;

	struct lws_client_connect_info i;
	struct lws *client_wsi;

	int counter;
	char finished;
	char established;
};

extern ws_sub_protocol_t ws_prot[WS_SUB_PROTOCOL_NUM];


void* test_callbake_msgHandle(void *arg);



#ifdef __cplusplus
}
#endif