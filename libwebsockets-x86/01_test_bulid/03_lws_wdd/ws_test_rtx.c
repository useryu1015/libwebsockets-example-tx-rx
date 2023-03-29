#include "ws_test_client.h"

/* create protocol cache*/
ws_sub_protocol_t ws_prot[WS_SUB_PROTOCOL_NUM];

static int ratelimit_connects(unsigned int *last, unsigned int secs)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
    
	if ((unsigned long)tv.tv_sec - (unsigned long)(*last) < (unsigned long)secs)
		return 0;

	*last = (unsigned int)tv.tv_sec;

	return 1;
}

/* fix: 改为单独的接收处理线程 */
void* test_thread(void *arg)
{
    ws_sub_protocol_t *vhd = (ws_sub_protocol_t *)arg;

    do {
        /* 测试 */
        // 隔3秒收发一次 垃圾数据.              结论: 最好用两个线程, 单独处理数据收发.  否则接收数据会出现被覆盖的情况
        int len = sprintf(vhd->p_user, "Test rubbish data from %s", vhd->prot_name);
        zlog_info("Send[%d]: %s", len, vhd->p_user);

        // vhd->user_state = USER_TX;
        lws_callback_on_writable(vhd->wsi_multi);

        sleep(1);
        lws_callback_on_writable(vhd->wsi_multi);

        // usleep(1000 * 1000);
        sleep(1);
    } while (vhd->pthread_state);

    pthread_exit(NULL);

    return 0;
}


// 接收线程
void* test_callbake_msgHandle(void *arg)
{
    ws_sub_protocol_t *vhd = (ws_sub_protocol_t *)arg;

    /* 测试线程, 随机发. 能否同步接收? */
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, test_thread, vhd);
    // pthread_detach(thread_id);

    do {
		if (!vhd->established || !vhd->wsi_multi)
			goto wait;

        /* 接收数据*/
        if (vhd->user_state == USER_RX)                     // LWS_CALLBACK_CLIENT_RECEIVE回调触发
        {
            zlog_info("Recv: %s", vhd->p_user);
            zlog_info("\n");

            memset(vhd->p_user, 0 , WS_TX_MAX_LEN);
            vhd->user_state = USER_NULL;
        }


        /* 发送数据*/
        if (vhd->user_state == USER_TX || 1)
        {
            static uint32_t limit;

            if (!ratelimit_connects(&limit, 2u))            // wait 2s
                goto wait;

            int len = sprintf(vhd->p_user, "This test_callbake_msgHandle from %s", vhd->prot_name);
            zlog_info("Send[%d]: %s", len, vhd->p_user);
            // memmove(vhd->p_user, "This test_callbake_msgHandle from %s", len);

            lws_callback_on_writable(vhd->wsi_multi);       // 触发LWS_CALLBACK_CLIENT_WRITEABLE回调
        }



wait:
        usleep(100 * 1000);
    } while (vhd->pthread_state);

    pthread_exit(NULL);
    
    return 0;
}



