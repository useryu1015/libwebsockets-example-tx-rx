#include "libwebsockets.h"
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
 
static volatile int exit_sig = 0;
#define MAX_PAYLOAD_SIZE  10 * 1024
#define MSGSZ 1024

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
};

/**
 * 数据收发 测试队列
 */
typedef struct msgbuf {
    long mtype;         // 消息类型
    char mtext[MSGSZ];  // 消息内容
    struct session_data ws_data;
} message_buf;

/**
 * 会话数据 接口
*/
struct session_data ws_writeable = {0};



// char str_test[128] = "hello this i s lieabeakl test l";

/**
 * 某个协议下的连接发生事件时，执行的回调函数
 *
 * wsi：指向WebSocket实例的指针
 * reason：导致回调的事件
 * user 库为每个WebSocket会话分配的内存空间
 * in 某些事件（接收缓存区）使用此参数，作为传入数据的指针
 * len 某些事件使用此参数，说明传入数据的长度
 */
int callback( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len ) {


    // struct session_data *data = (struct session_data *) user;
    struct session_data *data = (struct session_data *) &ws_writeable;


    switch ( reason ) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:   // 连接到服务器后的回调
            lwsl_notice( "Connected to server ok!\n" );
            break;
 
        case LWS_CALLBACK_CLIENT_RECEIVE:       // 接收到服务器数据后的回调，数据为in，其长度为len
            lwsl_notice( "Rx: %s\n", (char *) in );
            break;
        case LWS_CALLBACK_CLIENT_WRITEABLE:     // 当此客户端可以发送数据时的回调
            if ( 0 ) {
                // 如果有发送字节限制，则返回正数；如果无发送字节限制，则返回-1。
                printf("lws_get_peer_write_allowance运行发送字节数 %d \n", lws_get_peer_write_allowance(wsi));
                // 如果ws连接阻塞，则返回1，否则返回0。
                printf("lws_send_pipe_choked判断ws连接是否阻塞 %d \n", lws_send_pipe_choked(wsi));
            }

            if ( data->msg_count < 3 ) {
                // 前面LWS_PRE个字节必须留给LWS     // 协议封装帧头，且后面也需要预留
                // memset( data->buf, 0, sizeof( data->buf ));
                char *msg = (char *) &data->buf[ LWS_PRE ];
                data->len = sprintf( msg, "1 你好 %d", ++data->msg_count );
                lwsl_notice( "Tx: %s\n", msg );
                
                
                // 通过WebSocket发送文本消息
                lws_write( wsi, &data->buf[ LWS_PRE ], data->len, LWS_WRITE_TEXT );
			    // n = lws_write(wsi, &buf[LWS_PRE], block_size, opts | LWS_WRITE_TEXT);
            
            }
            // printf("user p: %p  ws_writeable p: %p   data p: %p\n", (struct session_data *)user, &ws_writeable, data);
            break;
        default:
            printf("1    callbake reason ohter: %d  \n", reason);
            printf("1     default    user p: %p  ws_writeable p: %p   data p: %p\n", (struct session_data *)user, &ws_writeable, data);
            break;
    }
    return 0;
}



int callback2( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len ) {


    // struct session_data *data = (struct session_data *) user;
    struct session_data *data = (struct session_data *) &ws_writeable;


    switch ( reason ) {
        case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
            printf("2   LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION user p: %p  ws_writeable p: %p   data p: %p\n", (struct session_data *)user, &ws_writeable, data);
            break;
        case LWS_CALLBACK_CLIENT_ESTABLISHED:   // 连接到服务器后的回调
            lwsl_notice( "Connected to server ok!\n" );
            printf("2   LWS_CALLBACK_CLIENT_ESTABLISHED user p: %p  ws_writeable p: %p   data p: %p\n", (struct session_data *)user, &ws_writeable, data);
            break;
        case LWS_CALLBACK_CLIENT_RECEIVE:       // 接收到服务器数据后的回调，数据为in，其长度为len      // 注意：指针in的回收、释放始终由LWS框架管理，只要出了回调函数，该空间就会被LWS框架回收。
            lwsl_notice( "Rx: %s\n", (char *) in );
            printf("2   LWS_CALLBACK_CLIENT_RECEIVE user p: %p  ws_writeable p: %p   data p: %p\n", (struct session_data *)user, &ws_writeable, data);
            break;
        case LWS_CALLBACK_CLIENT_WRITEABLE:     // 当此客户端可以发送数据时的回调
            if ( 0 ) {
                // 如果有发送字节限制，则返回正数；如果无发送字节限制，则返回-1。
                printf("2   lws_get_peer_write_allowance运行发送字节数 %d \n", lws_get_peer_write_allowance(wsi));
                // 如果ws连接阻塞，则返回1，否则返回0。
                printf("2   lws_send_pipe_choked判断ws连接是否阻塞 %d \n", lws_send_pipe_choked(wsi));
            }

            /**
             * 通过context获取用户指针
            */
            // struct lws_context *session = wsi->a.context;            // fix：错误：提领指向不完全类型的指针？？ 
            struct lws_context *session = lws_get_context(wsi);         // 1. 获取wsi中的context会话
            char *context_user = lws_context_user(session);             // 2. 获取context初始化配置的user用户指针
            printf(" \n\n2   context_user: %s \n\n\n", context_user);      // 3. 处理用户指针



            if ( data->msg_count < 3 ) {
                // 前面LWS_PRE个字节必须留给LWS     // 协议封装帧头，且后面也需要预留
                // memset( data->buf, 0, sizeof( data->buf ));
                char *msg = (char *) &data->buf[ LWS_PRE ];
                data->len = sprintf( msg, "2 你好 %d", ++data->msg_count );
                lwsl_notice( "2   Tx: %s\n", msg );
                
                
                // 通过WebSocket发送文本消息
                lws_write( wsi, &data->buf[ LWS_PRE ], data->len, LWS_WRITE_TEXT );
			    // n = lws_write(wsi, &buf[LWS_PRE], block_size, opts | LWS_WRITE_TEXT);
            
            }
            if ( 0 ) {
                // printf("\n\nuser p: %p  ws_writeable p: %p   data p: %p \n\n\n", (char *)user, str_test, data);
                // // printf("user p: %p  ws_writeable p: %p   data p: %p\n", (struct session_data *)user, &ws_writeable, data);
                // printf("user %s\n" , (char *)user);
                // printf("str_test %s\n", str_test);
            }
            break;

        case LWS_CALLBACK_CLOSED:
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            //连接失败或者中断,置空,便于重新连接 重新拉起
            printf(" \n\n2   连接失败或者中断  \n\n");
        default:
            printf("2    callbake reason ohter: %d  \n", reason);
            printf("2     default    user p: %p  ws_writeable p: %p   data p: %p\n", (struct session_data *)user, &ws_writeable, data);
            break;
    }
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
        "ws-asdsad", callback, sizeof( struct session_data ), 0, 0, NULL, 0            // fix: 如何传入user指针。
        // { NULL, NULL, 0, 0, 0, NULL, 0 }
    },    
    {
        //协议名称，协议回调，接收缓冲区大小
        "http-test", callback2, sizeof( struct session_data ), 0, 0, NULL, 0            // fix: 如何传入user指针。
        // { NULL, NULL, 0, 0, 0, NULL, 0 }
    },
    {
        NULL, NULL,   0 // 最后一个元素固定为此格式
    },
    LWS_PROTOCOL_LIST_TERM
};
 
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

    char str_test[128] = "hello this i s lieabeakl test l";
    ctx_info.user = str_test;               // fix： how to set user
    
    //ssl支持（指定CA证书、客户端证书及私钥路径，打开ssl支持）
    // ctx_info.ssl_ca_filepath = "../ca/ca-cert.pem";
    // ctx_info.ssl_cert_filepath = "./client-cert.pem";
    // ctx_info.ssl_private_key_filepath = "./client-key.pem";
    // ctx_info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
 
    // 创建一个WebSocket处理器
    // 1. 创建会话
    struct lws_context *context = lws_create_context( &ctx_info );
    if (!context) {
    // 创建失败，处理错误
    }
     char address[] = "172.17.68.125";
    // int port = 10100;
    int port = 8101;
    // char address[] = "127.0.0.1";
    // char address[] = "121.40.165.18"; 
    // int port = 8800;
    // char address[] = "192.168.101.73";     
    // int port = 8000;
    char addr_port[256] = { 0 };
    sprintf(addr_port, "%s:%u", address, port & 65535 );

    // 客户端连接参数
    struct lws_client_connect_info conn_info = { 0 };
    conn_info.context = context;
    conn_info.address = address;
    conn_info.port = port;
    conn_info.ssl_connection = 0    ;           // fix:  关闭了ssl
    conn_info.path = "./";
    conn_info.host = addr_port;
    conn_info.origin = addr_port;
    conn_info.protocol = protocols[ 0 ].name;   // 配置请求注册的协议， 未注册成功协议回调将卡在 reason==71;
 
    /**
     * 2. 连接服务器（通过i配置 lws_client_connect_info）
     *      依次触发回调reason： 27, 85, 105, 19, 31, 71, 29, 24, 71, 44, 2
    */
    struct lws *wsi = lws_client_connect_via_info( &conn_info );
    printf(" test 0  \n");

    // struct lws_context *test = wsi;
    // char *str = lws_context_user(context);
    // printf(" \n\n str: %s \n\n\n", str);

    while (1) {

        // strcpy(sbuf.mtext, "Hello, message queue!");
        // buflen = strlen(sbuf.mtext) + 1;
        // if (msgsnd(msqid, &sbuf, buflen, IPC_NOWAIT) == -1) {
        //     perror("msgsnd error");
        //     exit(1);
        // }
        
        // 客户端 服务调度程序  （心跳程序，必须轮询调用使能lws业务）
        lws_service( context, 0 );

        // lws_set_user

        // if (ws_writeable.msg_count > 5 || 1) {
        //     memset( ws_writeable.buf, 0, sizeof( ws_writeable.buf ));
        //     char *msg = (char *) &ws_writeable.buf[ LWS_PRE ];
        //     ws_writeable.len = sprintf( msg, "你好a 2178333333333333333333333333333333333172642164897212821964 %d", ++ws_writeable.msg_count );
        //     // printf("发送成功数据长度：%d  val:%s  \n", ws_writeable.len, &ws_writeable.buf[ LWS_PRE ]);            
        //     // ws_writeable.buf[ LWS_PRE ];
        //     int rlen = lws_write( wsi, &ws_writeable.buf[ LWS_PRE ], ws_writeable.len, LWS_WRITE_TEXT );

        //     // printf("发送成功数据长度：%d  val:%s  \n", rlen, &ws_writeable.buf[ LWS_PRE ]);
        // }

        // struct ws {
        //     int bool;
        //     user_buf;
        // }

        // 加入写入事件，并触发LWS_CALLBACK_CLIENT_WRITEABLE回调？      lws_service检测lws是否可写?
        lws_callback_on_writable( wsi );


        sleep(1);
    }

    // 销毁上下文对象
    lws_context_destroy( context );
    lwsl_user("Completed\n");
 
    return 0;
}
