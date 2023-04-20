libdir=../
#gcc -o -g server server.c -I$libdir/include -L$libdir/lib -lwebsockets



# # 测试服务端
# gcc -o -g server server.c -I../include -L../lib -lwebsockets
gcc server.c -o server -I../include -L../lib -lwebsockets -g

# # 测试客户端
gcc -o client client.c -I../include -L../lib -lwebsockets


# # 测试队列
# gcc -o queue queue.c 


# 封装 客户端
# gcc -o srv_client srv_websocket_client.c -I../include -L../lib -lwebsockets


