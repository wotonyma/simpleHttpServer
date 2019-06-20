//FCGI.h

#ifndef FCGI_H_
#define FCGI_H_

#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

#include <string>
#include <iostream>
using std::string;

#include <unistd.h>
#include <stdio.h>

#include <cstring>
#include <assert.h>
#include <memory.h>

#include <arpa/inet.h>

#include "fast_cgi.h"





class FCGI
{
public:
	FCGI();
	~FCGI();

	/* 连接php-fpm服务器 */
	void start_connect(void);

	/* 通用的构建头部的方法 */
	FCGI_Header make_header(int type, int requestId, int contentLength, int paddingLength);

	/* 构建开始请求时的body */
	FCGI_BeginRequestBody make_begin_request_body(int role, int aliveFlags);

	/* 构建name-value对的body */
	void make_name_value_pair_body(std::string name, int nameLen,
	                               std::string value, int valueLen,
	                               unsigned char *bodybuffer, int *bodyLen);

	/* 构建结束请求时的body */
	//FCGI_EndRequestBody make_end_request_body(int status);服务器不需要向php-fpm发送结束body,可以直接发送头部

	/* 发送开始请求record */
	void send_begin_request_record(void);

	/* 发送name-value对的record */
	void send_params_record(std::string name, std::string value);

	/* 发送空的name-value对record表明结束请求 */
	void send_empty_params_record(void);

	/* 发送stdin的请求record,type=5用于提交post表单数据 */
	void send_stdin_record(struct evbuffer* recv_evb);

	/* 发送空的stdin表明请求结束 */
	void send_empty_stdin_record(void);

	/* 读取php-fpm的响应内容 */
	bool read_from_php_fpm(struct evbuffer *evb);


private:
	evutil_socket_t sockfd; //与php-fpm连接的套接字
	
	static const int PARAMS_BUFF_LEN = 1024; //环境参数buffer的大小
    static const int CONTENT_BUFF_LEN = 1024;//内容buffer的大小
	
};

#endif
