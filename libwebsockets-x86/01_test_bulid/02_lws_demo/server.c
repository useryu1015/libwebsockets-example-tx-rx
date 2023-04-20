#include "libwebsockets.h"
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

static volatile int exit_sig = 0;
#define MAX_PAYLOAD_SIZE  10 * 1024

void sighdl( int sig ) {
    lwsl_notice( "%d traped", sig );
    exit_sig = 1;
}

/**
 * 会话上下文对象，结构根据需要自定义
 */
struct session_data {
    int msg_count;
    unsigned char buf[LWS_PRE + MAX_PAYLOAD_SIZE];
    int len;
    bool bin;
    bool fin;
};

static int protocol_my_callback( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len ) {
    struct session_data *data = (struct session_data *) user;
    switch ( reason ) {
        case LWS_CALLBACK_ESTABLISHED:       // 当服务器和客户端完成握手后
            printf("Client connect!\n");
            break;
        case LWS_CALLBACK_RECEIVE:           // 当接收到客户端发来的帧以后
            // 判断是否最后一帧
            data->fin = lws_is_final_fragment( wsi );
            // 判断是否二进制消息
            data->bin = lws_frame_is_binary( wsi );
            // 对服务器的接收端进行流量控制，如果来不及处理，可以控制之
            // 下面的调用禁止在此连接上接收数据
            lws_rx_flow_control( wsi, 0 );
 
            // 业务处理部分，为了实现Echo服务器，把客户端数据保存起来
            memcpy( &data->buf[ LWS_PRE ], in, len );
            data->len = len;
            printf("recvied message:%s\nend \n",in);
 
            // 需要给客户端应答时，触发一次写回调
            lws_callback_on_writable( wsi );
            break;
        case LWS_CALLBACK_SERVER_WRITEABLE:   // 当此连接可写时
            lws_write( wsi, &data->buf[ LWS_PRE ], data->len, LWS_WRITE_TEXT );
            // 下面的调用允许在此连接上接收数据
            lws_rx_flow_control( wsi, 1 );
            break;
    }
    // 回调函数最终要返回0，否则无法创建服务器
    return 0;
}

/**
 * 支持的WebSocket子协议数组
 * 子协议即JavaScript客户端WebSocket(url, protocols)第2参数数组的元素
 * 你需要为每种协议提供回调函数
 */
struct lws_protocols protocols[] = {
    {
        //协议名称，协议回调，接收缓冲区大小
        "protocol_test_callback", protocol_my_callback, sizeof( struct session_data ), MAX_PAYLOAD_SIZE,
    },
    {
        //协议名称，协议回调，接收缓冲区大小
        "test_callback", protocol_my_callback, sizeof( struct session_data ), MAX_PAYLOAD_SIZE,
    },
    {
        NULL, NULL,   0 // 最后一个元素固定为此格式
    },
    LWS_PROTOCOL_LIST_TERM
};
 
int main(int argc,char **argv)
{
    // 信号处理函数
    signal( SIGTERM, sighdl );
 
    struct lws_context_creation_info ctx_info = { 0 };
    ctx_info.port = 8000;
    ctx_info.iface = NULL; // 在所有网络接口上监听
    ctx_info.protocols = protocols;
    ctx_info.gid = -1;
    ctx_info.uid = -1;
    ctx_info.options = LWS_SERVER_OPTION_VALIDATE_UTF8;

    // fix:  2023年3月17日 ca报错
    // ctx_info.ssl_ca_filepath = "../ca/ca-cert.pem";
    // ctx_info.ssl_cert_filepath = "./server-cert.pem";
    // ctx_info.ssl_private_key_filepath = "./server-key.pem";
    // ctx_info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    //ctx_info.options |= LWS_SERVER_OPTION_REQUIRE_VALID_OPENSSL_CLIENT_CERT;
    
    struct lws_context *context = lws_create_context(&ctx_info);
    while ( !exit_sig ) {
        lws_service(context, 1000);
    }
    lws_context_destroy(context);

    return 0;
}
