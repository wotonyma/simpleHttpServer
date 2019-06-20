//HttpResponse.h

#ifndef HTTPRESPONSE_H_
#define HTTPRESPONSE_H_

/* libevent */
#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

/* c++ */
#include <string>
#include <iostream>
#include <sstream>
#include <utility>
#include <unordered_map>
using std::unordered_map;
using std::string;
using namespace std;

/* linux c */
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>

/* 自定义 */
#include "http.h"
#include "HttpRequest.h"
#include "FCGI.h"

class HttpResponse
{
public:
	 HttpResponse(HttpRequest *_request);
	~HttpResponse();

	void set_version();
	string & get_version() const;

	void set_code();
	int get_code();

	void set_code_str();
	string & get_code_str();

	void set_content_length(int _len);
	int get_content_length();



	void make_response_line(struct evbuffer *out);
	void make_response_head(struct evbuffer *out);
	void add_response_file(struct evbuffer *out);

	void make_response(struct evbuffer *out);
	void make_response_nocgi(struct evbuffer *out);
	void make_response_cgi(struct evbuffer *out);

private:
	string m_version;
	int m_code;
	string m_code_str;

	int m_content_length;   //发送正文的长度
	string m_suffix_type;   //后缀的种类

	HttpRequest *m_http_request; //解析类的指针,以此获得解析信息

	//存储发送内容主体的缓冲区
	struct evbuffer *m_send_evb;
	
	
};

#endif