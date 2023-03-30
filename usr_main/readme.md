**基于lws测试程序**


## lws-client describe
### lws启动流程
``` C
lws_create_context(); // 创建WebSocket会话接口
lws_client_connect_via_info(); // 连接服务器， 并触发LWS_CALLBACK_PROTOCOL_INIT回调
lws_service(); // lws服务调度（心跳程序，必须轮询调用使能lws业务）
```
### lws功能函数
``` C
lws_callback_on_writable();   // 1. 加入写入事件 触发LWS_CALLBACK_CLIENT_WRITEABLE回调
lws_callback_on_writable_all_protocol();  
lws_write();                  // 2. 向对端发送数据

lws_get_context(wsi);            // 1. 获取wsi中的context会话
lws_context_user(lws_context);   // 2. 获取context指向的user指针

lws_get_peer_write_allowance(wsi);// 如果有发送字节限制，则返回正数；如果无发送字节限制，则返回-1。
lws_send_pipe_choked(wsi);// 如果ws连接阻塞，则返回1，否则返回0。
```
### test业务程序
``` C
/* init */
test_callback();    // 
ws_prot_regist();   // 业务子协议栈注册
ws_prot_pthread_creat();

/* run */
test_callbake_msgHandle(); // 数据收发测试，业务线程
ws_send_msg();       // 向ws对端发送数据
```


## lws-server describe
### 启动流程
``` C
lws_create_context(); // 创建WebSocket会话接口

```




## tips
1. 不能在lws_service()处做阻塞操作, 影响lws业务调度效率; 回调中同理?
2. 

## fix
1. lwsl_notice改为zlog
2. test_callbake_msgHandle中:
   1. 验证 发送数据时, 触发RX回调的情况.
   2. 发送数据失败的处理, 重发等.
   3. 
3. end