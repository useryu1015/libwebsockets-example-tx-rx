#include "ws_test_client.h"

static volatile int exit_sig = 0;

void sighdl( int sig ) {
    zlog_info( "%d traped", sig );
    exit_sig = 1;
}

/**
 * 会话测试数据 接口
*/
struct session_data ws_writeable = {0};

# define LOG_TEST   \
    if ( user ) {   \
        zlog_warn("LOG_TEST: %s", __func__);  \
        zlog_warn(" calllback other reason %d  ", reason);    \
    }


inline int _ws_send_msg(struct lws *wsi, uint8_t * msg, int len)
{
    int rlen;

    if ( !msg ) {
        zlog_info("LWS send error");
        return -1;
    }

    unsigned char sbuf[LWS_PRE + WS_TX_MAX_LEN] = {0};                      // 可以规定入口参数必须预留LWS_PRE 减去拷贝消耗
    memmove(&sbuf[ LWS_PRE ], msg, len);

    zlog_info("send[%d]: %s", len, &sbuf[ LWS_PRE ]);
    rlen = lws_write( wsi, &sbuf[ LWS_PRE ], len, LWS_WRITE_BINARY );         // event LWS_CALLBACK_CLIENT_WRITEABLE 

    return rlen;
}


static int ratelimit_connects(unsigned int *last, unsigned int secs)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	if ((unsigned long)tv.tv_sec - (unsigned long)(*last) < (unsigned long)secs)
		return 0;

	*last = (unsigned int)tv.tv_sec;

	return 1;           // 允许重连
}


/* 获取子协议vhost*/
ws_sub_protocol_t * ws_get_substack()
{
    int n;

    for (n = 0; n < WS_SUB_PROTOCOL_NUM; n++)
    {
        ;
    }
    return NULL;
}


/**
 * 某个协议下的连接发生事件时，执行的回调函数
 * wsi：指向WebSocket实例的指针
 * reason：导致回调的事件
 * user 库为每个WebSocket会话分配的内存空间
 * in 某些事件使用此参数，作为传入数据的指针
 * len 某些事件使用此参数，说明传入数据的长度
 */
int protocol_test_callback( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len ) 
{
    ws_sub_protocol_t *vhd = &ws_prot[PROTOCOL_TEST_CALLBACK];   // ws_get_substack(lws_get_protocol(wsi), __func__);
    char *pusr = (char *)user;
    int rtn;

    switch ( reason ) {
        case LWS_CALLBACK_PROTOCOL_INIT:        // lws初始化时
            vhd->pthread_state = PTHREAD_INIT;
            break;
        case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
            break;
        case LWS_CALLBACK_CLIENT_ESTABLISHED:   // 连接到服务器成功
            zlog_trace(" %s:Connected to server ok!", __func__);
		    vhd->established = 1;
            break;
        case LWS_CALLBACK_CLIENT_RECEIVE:       // 接收到服务器数据后的回调，数据为in，其长度为len      // 注意：指针in的回收、释放始终由LWS框架管理，只要出了回调函数，该空间就会被LWS框架回收。
            // 判断是否最后一帧
            ;int fin = lws_is_final_fragment( wsi );
            // 判断是否二进制消息
            int bin = lws_frame_is_binary( wsi );
            // 对服务器的接收端进行流量控制，如果来不及处理，可以控制之
            // 下面的调用禁止在此连接上接收数据
            lws_rx_flow_control( wsi, 0 );
            // 下面的调用允许在此连接上接收数据
            lws_rx_flow_control( wsi, 1 );

            zlog_trace( "Rx: %s", (char *) in );            
            
            pthread_mutex_lock(&vhd->lock_ring); /* --------- ring lock { */
            memset(vhd->p_user, 0 , WS_TX_MAX_LEN);
            memmove(pusr, in, len);
            
            vhd->user_len = len;
            vhd->user_state = USER_RX;

            pthread_mutex_unlock(&vhd->lock_ring); /* } ring lock */
            break;
        case LWS_CALLBACK_CLIENT_WRITEABLE:     // 当此客户端可以发送数据时的回调
            pthread_mutex_lock(&vhd->lock_ring); /* --------- ring lock { */
            rtn = _ws_send_msg(wsi, pusr, strlen(pusr));
            if (rtn < strlen(pusr)) {
                pthread_mutex_unlock(&vhd->lock_ring); /* } ring lock */
                zlog_error("ERROR %d writing to ws socket", rtn);
                return -1;
            }

            pthread_mutex_unlock(&vhd->lock_ring); /* } ring lock */
            break;
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		    zlog_error("CLIENT_CONNECTION_ERROR: %s ",
			 in ? (char *)in : "(null)");        
        case LWS_CALLBACK_CLIENT_CLOSED:        
            // 置空,便于重新连接 重新拉起
            zlog_error("连接关闭");
            vhd->wsi_multi = NULL;      // 断开后lws自动销毁
		    vhd->established = 0;
            vhd->pthread_state = 0;
            pthread_join(vhd->pthread_spam, NULL);
            break;
        default:
            LOG_TEST
            break;
    }
    return 0;
}

int callback2( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len ) {
    ws_sub_protocol_t *vhd = &ws_prot[PROTOCOL_TEST_CALLBACK];   // ws_get_substack(lws_get_protocol(wsi), __func__);
    char *pusr = (char *)user;
    int rtn;
    
    switch ( reason ) {
        case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
            break;
        case LWS_CALLBACK_CLIENT_ESTABLISHED:   // 连接到服务器后的回调
            zlog_info( "Connected to server ok! " );
            break;
        case LWS_CALLBACK_CLIENT_RECEIVE:       // 接收到服务器数据后的回调，数据为in，其长度为len      // 注意：指针in的回收、释放始终由LWS框架管理，只要出了回调函数，该空间就会被LWS框架回收。
            zlog_info( "Rx: %s ", (char *) in );
            break;
        case LWS_CALLBACK_CLIENT_WRITEABLE:     // 当此客户端可以发送数据时的回调
            // sleep(5);           // ques: 验证 回调做阻塞操作的影响
            // zlog_info("lws_get_peer_write_allowance运行发送字节数 %d ", 
            //         lws_get_peer_write_allowance(wsi));// 如果有发送字节限制，则返回正数；如果无发送字节限制，则返回-1。
            // zlog_info("lws_send_pipe_choked判断ws连接是否阻塞 %d ", 
            //         lws_send_pipe_choked(wsi));// 如果ws连接阻塞，则返回1，否则返回0。
            
            /* 通过context获取用户指针*/
            // struct lws_context *session = wsi->a.context;                // fix：错误：提领指向不完全类型的指针？？ 
            // struct lws_context *session = lws_get_context(wsi);          // 1. 获取wsi中的context会话
            // char *context_user = (char *)lws_context_user(session);      // 2. 获取context初始化配置的user用户指针
            // zlog_info("1   context_user: %s ", context_user);            // 3. 处理用户指针
            // zlog_info("context_user:%p  ws_write:%p ", context_user, ws_writeable.buf);
            LOG_TEST
            break;
        case LWS_CALLBACK_CLOSED:
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            //连接失败或者中断,置空,便于重新连接 重新拉起
            zlog_info("2   连接失败或者中断   ");
        default:
            LOG_TEST
            break;
    }
    return 0;
}

int srv_callback( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len ) {

    switch ( reason ) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:   // 连接到服务器后的回调
            zlog_info( "Connected to server ok! " );
            break;
        case LWS_CALLBACK_CLIENT_RECEIVE:       // 接收到服务器数据后的回调，数据为in，其长度为len
            zlog_info( "Rx: %s ", (char *) in );
            break;
        case LWS_CALLBACK_CLIENT_WRITEABLE:     // 当此客户端可以发送数据时的回调
            LOG_TEST;
            break;
        default:
            break;
    }
    return 0;
}


/**************************     自定义协议栈初始化程序  ws_protocols_inti.c     ************************************/
/**
 * list of supported protocols and callbacks
 * 支持的WebSocket自定义子协议数组
 *  1. 你需要为每种协议提供回调函数
 *  2. 调用lws_client_connect_via_info(i);函数，配置 i->protocol == lws_protocols->name 注册子协议到lws服务端
 */
static const struct lws_protocols protocols[] = {
	{
		// "protocol_test_callback",       // 协议的名称
		"test_callback",       // 协议的名称
		// "",       // 协议的名称
		protocol_test_callback,                  // 对应的回调函数
		sizeof( struct session_data ),  // user堆栈大小； 在子协议初始化成功后lws内核才会分配内存
        4096,                           // 接收缓存区大小
        0, 
        NULL,                           // 无实意，通过lws_client_connect_info结构体的userdata配置默认指针
        0
	}, {
		"lws-mirror-protocol",
		callback2,
		128, 0, 0, NULL, 0
	}, {
		"lws-test-raw-client",
		srv_callback,
		0, 0, 0, NULL, 0
	},
	LWS_PROTOCOL_LIST_TERM
};


/**
 * 初始化ws子协议
 *  通过lws_client_connect_via_info() 连接到服务端，注册ws协议栈
*/
int ws_prot_regist(ws_sub_protocol_t *p_wsi, struct lws_client_connect_info *i)
{
    int n;

    if (!i && !p_wsi)
        return -1;

    for (n = 0; n < WS_SUB_PROTOCOL_NUM && n<1; n++) {
    // for (n = 0; n < WS_SUB_PROTOCOL_NUM; n++) {
        if (!p_wsi[n].wsi_multi /*&& ratelimit_connects(&p_wsi[n].rl_multi, 2u)*/) {
            p_wsi[n].prot_name = protocols[n].name;
            p_wsi[n].p_user = malloc(LWS_PRE + WS_TX_MAX_LEN);

            i->protocol = protocols[n].name;                // 配置ws子协议，后续将请求注册到服务端中
            i->pwsi = &p_wsi[n].wsi_multi;                  // store the new wsi here early in the connection process
            i->userdata = p_wsi[n].p_user;                  // 配置回调函数user指针
            p_wsi[n].conn_info = i;
            // protocols[n].user = p_wsi[n].p_user;
            // i->userdata = protocols[n].user;

            /**
             * 连接服务器
             *  依次触发回调reason： 27, 85, 105, 19, 31, 71, 29, 24, 71, 44, 2
            */
            lws_client_connect_via_info(i);

            zlog_trace("protocols %d-%s: connecting ", n, p_wsi[n].prot_name);
            // p_wsi->protocol_num++;
        }
    }

#ifdef USE_WS_HTTP
    if (!p_wsi->wsi_dumb && ratelimit_connects(&p_wsi->rl_dumb, 2u)) {
        i->protocol = NULL;
        i->pwsi = &p_wsi->wsi_dumb;
        zlog_info("http: connecting [%s] ", i->protocol);
        
        lws_client_connect_via_info(i);
    }
#endif

    return 0;
}

/**
 * 初始化ws子协议业务线程
*/
int ws_prot_pthread_create(ws_sub_protocol_t *p_wsi)
{
    int n;

    if (!p_wsi)
        return -1;

    for (n = 0; n < WS_SUB_PROTOCOL_NUM; n++)              // fix: 同一IP注册
    {
        if (!p_wsi[n].wsi_multi /*&& p_wsi[n].pthread_state*/)
            continue;

        switch (n)  // fix: 定位方式
        {
        case PROTOCOL_TEST_CALLBACK:
        case PROTOCOL_LWS_MIRROR:
        case DEMO_PROTOCOL_COUNT:
            pthread_mutex_init(&p_wsi[n].lock_ring, NULL);

            if(pthread_create(&p_wsi[n].pthread_spam, NULL, 
                                test_callbake_msgHandle, &p_wsi[n])) {
                zlog_error("thread creation failed");

                /* 结束线程*/
                p_wsi[n].pthread_state = PTHREAD_FILISH;
                pthread_join(p_wsi[n].pthread_spam, NULL);
                pthread_mutex_destroy(&p_wsi[n].lock_ring);
            }
            break;
        default:
            zlog_error("No such protocol!");
            break;
        }
        
        zlog_trace("protocols %d-%s: pthread create ", n, p_wsi[n].prot_name);

    }

    return 0;
}

/**
 * 客户端 重连
*/
int ws_client_monitor(ws_sub_protocol_t *p_wsi)
{
    int n;

    if (!p_wsi)
        return -1;

    for (n = 0; n < WS_SUB_PROTOCOL_NUM && n<1; n++)              // fix: 同一IP注册
    {
        if (!p_wsi[n].wsi_multi && ratelimit_connects(&p_wsi[n].rl_multi, 5u)) {
            zlog_trace("%s reconnect", p_wsi[n].prot_name);

            ws_prot_regist(p_wsi, p_wsi[n].conn_info);
            ws_prot_pthread_create(p_wsi);
        }
    }

    return 0;
}

int main() {
    // 信号处理函数
    signal( SIGTERM, sighdl );
 
    // 用于创建vhost或者context的参数
    struct lws_context_creation_info ctx_info = { 0 };
    ctx_info.port = CONTEXT_PORT_NO_LISTEN;
    ctx_info.iface = NULL;
    ctx_info.protocols = protocols;
    ctx_info.gid = -1;
    ctx_info.uid = -1;

    // memmove(ws_writeable.buf, "hello im writeabel", 128);
    // ctx_info.user = ws_writeable.buf;           // 全局指针
 
    struct lws_context *context = lws_create_context( &ctx_info );

    // char address[] = "121.40.165.18";
    // int port = 8800;
    // char address[] = "192.168.101.73";
    // int port = 8000;
    /* http://172.17.68.125:8101/  这个是公司服务器部署demo  */
    char address[] = "172.17.68.125";
    // char address[] = "192.168.101.33";
    int port = 10100;
    // int port = 8101;
    char addr_port[256] = { 0 };
    sprintf(addr_port, "%s:%u", address, port & 65535 );

    // 客户端连接参数
    struct lws_client_connect_info conn_info = { 0 };
    conn_info.context = context;                // 
    conn_info.address = address;
    conn_info.port = port;
    conn_info.ssl_connection = 0;               // 关闭ssl
    conn_info.path = "websocket";
    conn_info.host = addr_port;
    conn_info.origin = addr_port;


    // 2023年4月7日：  考虑重连
    ws_prot_regist(ws_prot, &conn_info);
    ws_prot_pthread_create(ws_prot);

    // lws初始化，创建线程调度
    while (!exit_sig) {
        lws_service(context, 0);

        // ws_client_monitor(ws_prot);         // 重连
        // usleep(10 * 1000);                  // 查看cpu占用 top       避免长阻塞操作
    }

    // 销毁上下文对象
    lws_context_destroy(context);
    zlog_trace("Completed ");
 
    return 0;
}
