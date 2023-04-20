/********************
 * websocket与sensor交互接口设计：
 *  1) 通过消息队列
 *  2) 封装ws接口, sensor直接调ws接口库
 * 
 * WS断开后重连
 * 
 * 
*/
#include "ws_test_client.h"

/**
 * 用户业务程序
*/
int srv_ws_test()
{



    /* 读取json数据*/


}







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


/**
 * 业务测试线程,处理WS数据收发， 业务说明：
 *  1. 处理传感器数据，转发到上位机
 *  2. 接收上位机数据，下发到传感器
 * 考虑问题：
 *  1. ws断开后重连，重连后数据丢失？
 *  2. 数据传输不丢包、稳定、快速
*/
void* test_callbake_msgHandle(void *arg)
{
    ws_sub_protocol_t *vhd = (ws_sub_protocol_t *)arg;
    vhd->pthread_state = PTHREAD_RUN;

    /* 测试线程, 随机发. 能否同步接收? */
    // pthread_t thread_id;
    // pthread_create(&thread_id, NULL, test_thread, vhd);
    // pthread_detach(thread_id);

    do {
		if (!vhd->established || !vhd->wsi_multi)
			goto wait;

        /**
         * 接收上位机数据
         *  如果上位机数据更新过快, 会被覆盖; 与上位机对接优化
         * */
        if (vhd->user_state == USER_RX)                     // LWS_CALLBACK_CLIENT_RECEIVE回调触发
        {
            zlog_info("Recv[%d]: %s \n\n", vhd->user_len, vhd->p_user);

            /**
             * 1.转发数据到sensor 队列 
             * 2.直接在此处操作sensor
             * */
            // queue_send

            memset(vhd->p_user, 0 , WS_TX_MAX_LEN);
            vhd->user_state = USER_NULL;
            // 下面的调用允许在此连接上接收数据
            lws_rx_flow_control(vhd->wsi_multi, 1);
        }


        /* 发送数据*/
        if (vhd->user_state == USER_TX || 1)
        {
            static uint32_t limit;

            if (!ratelimit_connects(&limit, 2u))            // test wait 2s
                goto wait;

            /* 处理sensor业务数据 接收队列消息 */
            // queue_recv

            // int len = sprintf(vhd->p_user, "{\"cmdCode\":1,\"cmdMerge\":65536,\"data\":[123,34,110,97,109,101,34,58,34,55,55,55,55,55,55,55,34,125]}");
            int len = sprintf(vhd->p_user, "{\"cmdCode\":1,\"cmdMerge\":65538,\"data\":[123,34,106,115,111,110,34,58,123,34,110,97,109,101,34,58,34,-25,-69,-120,-25,-85,-81,49,34,44,34,105,100,34,58,34,50,48,50,51,48,51,49,49,48,48,48,49,120,108,34,44,34,116,101,114,109,105,110,97,108,34,58,34,67,79,76,34,44,34,109,116,121,112,101,34,58,34,77,83,71,69,34,44,34,116,34,58,34,50,48,50,51,45,48,52,45,48,53,32,48,57,58,50,56,58,52,55,46,52,55,53,34,44,34,112,97,114,97,109,115,34,58,91,123,34,107,107,115,34,58,34,107,107,115,-25,-68,-106,-25,-96,-127,34,44,34,114,101,102,34,58,34,-27,-113,-111,-27,-113,-104,56,49,49,-27,-79,-126,47,-26,-100,-70,-25,-69,-124,49,46,-26,-80,-76,-27,-114,-117,49,34,44,34,118,97,108,34,58,34,48,46,53,54,45,77,112,97,34,44,34,116,34,58,34,50,48,50,51,45,48,52,45,48,53,32,48,57,58,50,56,58,52,55,46,56,55,50,34,125,93,125,125]}");
            // int len = sprintf(vhd->p_user, "{\"cmdCode\":1,\"cmdMerge\":65538,\"data\":[123,34,106,115,111,110,34,58,123,34,110,97,109,101,34,58,34,116,101,114,109,105,110,97,108,49,34,44,34,105,100,34,58,34,50,48,50,51,48,51,49,49,48,48,48,49,120,108,34,44,34,116,101,114,109,105,110,97,108,34,58,34,67,79,76,34,44,34,109,116,121,112,101,34,58,34,77,83,71,69,34,44,34,116,34,58,34,50,48,50,51,45,48,52,45,48,53,32,48,57,58,50,56,58,52,55,46,52,55,53,34,44,34,112,97,114,97,109,115,34,58,91,123,34,107,107,115,34,58,34,107,107,115,116,101,115,116,34,44,34,114,101,102,34,58,34,114,101,107,107,115,116,101,115,116,102,102,49,34,44,34,118,97,108,34,58,34,48,46,53,54,45,77,112,97,34,44,34,116,34,58,34,50,48,50,51,45,48,52,45,48,53,32,48,57,58,50,56,58,52,55,46,56,55,50,34,125,93,125,125]}");
            // int len = sprintf(vhd->p_user, "{\"cmdCode\":1,\"cmdMerge\":65537,\"data\":[]}");
            zlog_info("Send[%d]: %s", len, vhd->p_user);
            // memmove(vhd->p_user, "This test_callbake_msgHandle from %s", len);

            lws_callback_on_writable(vhd->wsi_multi);       // 触发LWS_CALLBACK_CLIENT_WRITEABLE回调
        }

wait:
        usleep(10 * 1000);
    } while (vhd->pthread_state);

    pthread_exit(NULL);
    
    return 0;
}



