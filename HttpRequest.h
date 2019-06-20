//HttpRequest.h
#ifndef HTTPREQUEST_H_
#define HTTPREQUEST_H_

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
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
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

#include "http.h"

class HttpRequest
{
public:
	HttpRequest();
	~HttpRequest();

	void set_http_method(string _method);    //设置方法
	const string get_http_method() const;  //得到方法

	void set_http_url(const string& _url);  //设置url
	string& get_http_url();           //得到url

	void set_http_version(const string& _version);   //设置version
	const string& get_http_version() const;          //得到version

	bool is_method_ok();
	bool request_line_parse(struct evbuffer *evb);      //请求行解析
	void url_parse();                               //url解析
	int file_stat_parse();                          //根据path解析文件状态
	void request_head_parse(struct evbuffer *evb);  //head_key_value
	bool is_need_recv_text();    //有无主体
	bool is_cgi();  //是否是cgi

	void set_http_path(string _path);
	string & get_http_path();

	void set_http_param(string _param);
	string & get_http_param();

	void set_http_resource_size(int _resource_size);
	int get_http_resource_size();

	void set_http_resource_suffix(string _resource_suffix);
	string & get_http_resource_suffix();

	void set_http_content_length(int _content_length);
	void set_http_content_length_on_kvmap();
	int get_http_content_length();

	std::unordered_map<std::string, std::string> & get_http_head_kvmap();

	int get_http_recv_data(struct evbuffer* evb); //将剩余读缓冲区的数据读取到缓冲区
	struct evbuffer* get_http_recv_evb();

	void set_code(int _code);
	int get_code();

	void all_request_parse(struct evbuffer* evb);

private:
	enum http_request_method m_method; //保存method
	string m_url;
	string m_version;
	string m_host;

	bool m_cgi; //是否是cgi

	string m_path;  //路径
	string m_param; //参数

	struct stat m_file_stat;  //文件属性

	int m_resource_size;      //请求资源的大小
    string m_resource_suffix; //请求资源的后缀
    int m_content_length;     //request主体Content-Length

    std::unordered_map<std::string, std::string> m_head_kvmap;   //head里key-value map

    struct evbuffer* m_recv_evb;   //存放http请求报文主体

    int m_code;

};

#endif