乌东德.md

## 参考文献
**lws官方例程**	
* build后的测试程序(build/bin/*)：	
	libwebsockets-test-client	
	libwebsockets-test-server	
* 数据收发demo程序：	
	minimal-examples-lowlevel/ws-client/minimal-ws-client-tx/minimal-ws-client.c	
	minimal-examples-lowlevel/ws-client/minimal-ws-client-spam-tx-rx/minimal-ws-client.c	
	minimal-examples-lowlevel/ws-client/minimal-ws-client-rx/minimal-ws-client.c	

**安装教程**
- [Git-libwebsockets](https://github.com/warmcat/libwebsockets)
- [libwebsockets简介](https://blog.csdn.net/u013780605/article/details/79489183)
- [测试环境搭建&源码讲解](https://blog.csdn.net/fantasysolo/article/details/88908948)
- [交叉编译](https://blog.csdn.net/lx121451/article/details/78717366)
- [交叉编译](https://blog.csdn.net/W75391/article/details/126172775)

**测试程序**
- [demo程序](https://blog.csdn.net/yuanwei1314/article/details/76228495)
- [demo程序，注释](https://blog.csdn.net/weixin_39510813/article/details/86728804)
- [服务端地址](http://192.168.101.73:7681/)

**接口函数**
- [C++封装lws](https://blog.csdn.net/weixin_39510813/article/details/86728916)

**其他**
- [网络协议 lws通信原理](https://blog.csdn.net/alan_liuyue/article/details/120822808)

- 搭建自己的ws server
linux下libwebsockets编译及实例：
https://blog.csdn.net/yuanwei1314/article/details/76228495
利用libwebsockets写ws、wss服务端和客户端：
https://blog.csdn.net/weixin_39510813/article/details/86728804
libwebsockets之简单服务器代码：
https://blog.csdn.net/qq_39101111/article/details/79025438

- 封装
参考：封装利用libwebsockets写出的客户端、服务端程序为客户端服务端类
https://blog.csdn.net/weixin_39510813/article/details/86728916
https://blog.csdn.net/qq_36972985/article/details/82897868

- API
部分转自：
总结使用libwebsockets开发接入层：https://blog.csdn.net/qifengzou/article/details/50281545 libwebsockets（三）实现简易websocket服务器：https://blog.csdn.net/u013780605/article/details/79489197
相关中文资料很少，感谢博主。结合说明对照自带的demo查看。



## 熟悉HTTP协议，  加深websocket网络通信原理
1. 粗通mqtt协议： 协议如何保证数据稳定？
	1）异步通信机制，2）服务器会为每个主题存储最新一条保留消息，3）基于TCP协议
2. openssl?? 

## 笔记
* ajax轮询：		定时请求数据，非阻塞等待响应
* long poll：		请求数据，并阻塞等待响应
* websocket：		全双工，主动请求数据， 被动接收数据， 
* http：			半双工，
	* HTTP1.0:		tcp短链接
	* HTTP1.1:		tcp长链接



## 进度
1. 交叉编译！
2. 库与源文件封装、打包
3. 交互数据：  
	.json文件 or json格式数据
	客户端与客户端通信 or 客户端与服务端通信？
	self：
		1. ws服务端作为数据库， 客户端进行数据的存取操作？
		2. 两客户端并不存在直接context会话， 间接通过存取‘数据库’到达共享数据
4. 客户端数据传输接口


## 主要函数
* lws-client.h			// 客户端函数

* lws_write
功能：将数据发送给对端
备注：函数参数说明	
  wsi: ws连接对象	
  buf: 需要发送数据的起始地址。	
	注意：必须在指针buf前预留长度为LWS_SEND_BUFFER_PRE_PADDING的空间，同时在指针buf+len后预留长度为LWS_SEND_BUFFER_POST_PADDING的空间。	
  len: 需要发送数据的长度	
  protocol: 如果该连接是http连接，则该参数的值为LWS_WRITE_HTTP；如果该连接是ws连接，则该参数的值为LWS_WRITE_BINARY，但如果第一次发送的数据长度n < len，则发送后续长度为(len - n)字节的数据时，该参数值改为LWS_WRITE_HTTP。	




## 数据收发demo
一：
	通过user配置用户数据指针， 业务传入数据buf

二：
	收发消息队列调度， 回调中对消息队列进行收发
	发送：
		1. 业务向队列写入数据
		2. 调用 lws_callback_on_writable( wsi ); 触发ws_write回调
		3. 回调函数读取队列数据， 并封装发送buf
	接收：
		1. 回调向队列写入数据
		2. 业务程序监听线程消息， 解析ws主动推送数据。

三：
	直接调用lws_write( wsi, &ws_writeable.buf[ LWS_PRE ], ws_writeable.len, LWS_WRITE_TEXT );
	问题： 绕过lws_callback_on_writable( wsi );调度， 可能不合理， 业务未连接成功时，出现段错误


[OPENDDS](https://baike.baidu.com/item/OPENDDS/979069?fr=aladdin)

通过轮询 数据队列是否有发送的消息与接收的消息
发送：
	队列pop消息，有数据时调用lws_callback_on_writable( wsi ); 使能LWS_CALLBACK_CLIENT_WRITEABLE回调
接收：
	反之，向接收队列写入数据


libwebsocket的客户端可以使用libwebsocket_write函数来发送数据。该函数的原型如下：

int libwebsocket_write(struct libwebsocket* wsi, unsigned char* buf, size_t len, enum libwebsocket_write_protocol protocol)
其中，wsi是libwebsocket实例的指针，buf是要发送的数据缓冲区的指针，len是要发送的数据长度，protocol是发送数据的协议类型。

例如，下面的代码演示了如何使用libwebsocket发送一条文本消息：

char* message = "Hello, libwebsocket!";
int len = strlen(message);
libwebsocket_write(wsi, (unsigned char*)message, len, LWS_WRITE_TEXT);
在上述代码中，wsi是libwebsocket实例的指针，message是要发送的文本消息的指针，len是消息的长度，LWS_WRITE_TEXT是消息的协议类型，表示发送的是文本消息。



## 交叉编译
### 编译问题
-- Configuring incomplete, errors occurred!
See also "/home/project_xlian/WDD/websocket/libwebsockets-main/build/CMakeFiles/CMakeOutput.log".
See also "/home/project_xlian/WDD/websocket/libwebsockets-main/build/CMakeFiles/CMakeError.log".

原因： 
$CC 配置为其他工具链				echo $CC

### 交叉编译lws客户端程序？ 
> 1. 打包或封装客户端源码， 移植到wdd代码中
> 2. 交叉编译动态库

cmake .. \
-DCMAKE_INSTALL_PREFIX=/usr/local/libwebs-arm \
-DCMAKE_SYSTEM_NAME=Linux \
-DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc  \
-DCMAKE_CXX_COMPILER=arm-linux-gnueabihf-g++  \
-DZLIB_INCLUDE_DIR=/usr/local/zlib-arm/include \
-DZLIB_LIBRARY="/usr/local/zlib-arm/lib/libz.so"  \
-DLWS_OPENSSL_INCLUDE_DIRS=/usr/local/ssl-arm/include/ \
-DLWS_OPENSSL_LIBRARIES="/usr/local/ssl-arm/lib/libssl.so;/usr/local/ssl-arm/lib/libcrypto.so"

arm-linux-gnueabihf-gcc

/opt/SCM801-linux/gcc-linaro-7.4.1-2019.02-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gccS
CC=/opt/SCM801-linux/gcc-linaro-7.4.1-2019.02-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc
/opt/SCM701-linux/gcc-linaro-4.9-2016.02-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc

### 英飞凌主板编译配置——  CMAKExx.txt
SET(CMAKE_SYSTEM_NAME Linux)
SET(TOOLCHAIN_DIR "/opt/fsl-imx-x11/4.1.15-2.0.0/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi")

SET(CMAKE_FIND_ROOT_PATH "${TOOLCHAIN_DIR}")
SET(CMAKE_C_COMPILER "/opt/fsl-imx-x11/4.1.15-2.0.0/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-gcc")
SET(CMAKE_CXX_COMPILER "/opt/fsl-imx-x11/4.1.15-2.0.0/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-g++")

set(MYSYSROOT /opt/fsl-imx-x11/4.1.15-2.0.0/sysroots/cortexa7hf-neon-poky-linux-gnueabi)

compiler/linker flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv7ve -marm -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a7 --sysroot=${MYSYSROOT}" CACHE INTERNAL "" FORCE)
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -march=armv7ve -marm -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a7 --sysroot=${MYSYSROOT}" CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --sysroot=${MYSYSROOT}" CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} --sysroot=${MYSYSROOT}" CACHE INTERNAL "" FORCE)

set(CMAKE_FIND_ROOT_PATH "${MYSYSROOT}")

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
#for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_C_FLAGS"${CMAKE_C_FLAGS} --sysroot=/opt/fsl-imx-x11/4.1.15-2.0.0/sysroots/cortexa7hf-neon-poky-linux-gnueabi")
SET(CMAKE_CXX_FLAGS"${CMAKE_CXX_FLAGS} --sysroot=/opt/fsl-imx-x11/4.1.15-2.0.0/sysroots/cortexa7hf-neon-poky-linux-gnueabi")

#set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
SET(CMAKE_C_COMPILER_WORKS TRUE)

SET(CMAKE_CXX_COMPILER_WORKS TRUE)

## 总则
	OBJ： 开发过程中，对程序的描述。为便于自己的回溯，文章有很多哆嗦的地方










