//HttpResponse.cpp

#include "HttpResponse.h"

HttpResponse::HttpResponse(HttpRequest *_request)
{
	m_http_request = _request;
	m_send_evb = evbuffer_new();
	m_version = m_http_request->get_http_version();
	m_code = m_http_request->get_code();
}

HttpResponse::~HttpResponse()
{
	evbuffer_free(m_send_evb);
}


void HttpResponse::make_response_line(struct evbuffer *out)
{
	//code映射codestr
	char *code_str = const_cast<char *>(http_response_phrase_internal(m_code));
	m_code_str = code_str; //保存codestr,有没有无所谓
	evbuffer_add_printf(out, "%s %d %s\n", m_version.c_str(), m_code, code_str);
}

//用于静态文件,动态文件不适合
void HttpResponse::make_response_head(struct evbuffer *out)
{
	
	
	//获取文件描述符,将文件暂时储存在evbuffer
	/**/
	string& whole_path = m_http_request->get_http_path();
	int fd;

	if ((fd = open(whole_path.c_str(), O_RDONLY)) < 0)
	{
		std::cout << "open file error" <<std::endl;
		m_code = 404;
	}

	if (m_code == 200)
	{
		int size = m_http_request->get_http_resource_size(); //获取之前解析的文件大小
		evbuffer_add_file(m_send_evb, fd, 0, size);

		m_content_length = evbuffer_get_length(m_send_evb);
		/* 可以进行错误判断m_content_length == size? */

		//suffix_type
		std::string &suffix = m_http_request->get_http_resource_suffix();
		m_suffix_type = stuffix_map[suffix];
		evbuffer_add_printf(out, "Content-Type: %s\n", m_suffix_type.c_str());
		//content_length
		evbuffer_add_printf(out, "Content-Length: %d\n", m_content_length);

		/* 根据需要添加其他key-value */
		evbuffer_add_printf(out, "\n");

	}
	else
	{
		char *_code_str = const_cast<char*>(http_response_phrase_internal(m_code));
		evbuffer_add_printf(m_send_evb,
                    "<HTML><HEAD>\n"
	    			"<TITLE>%d %s</TITLE>\n"
	    			"</HEAD><BODY>\n"
	    			"<H1>%s</H1>\n"
	    			"</BODY></HTML>\n",
		    		m_code,
		   			_code_str,
		    		_code_str);

		m_content_length = evbuffer_get_length(m_send_evb);
		evbuffer_add_printf(out, "Content-Type: text/html\n");
		evbuffer_add_printf(out, "Content-Length: %d\n", m_content_length);
		evbuffer_add_printf(out, "\n");
	}

	close(fd);


}

void HttpResponse::add_response_file(struct evbuffer *out)
{
	//把content发送
	evbuffer_add_buffer(out, m_send_evb);
}


void HttpResponse::make_response(struct evbuffer *out)
{
	//nocgi
	if(m_http_request->is_cgi() == false)
	{
		make_response_nocgi(out);
	}
	//cgi
	else
	{
		make_response_cgi(out);
	}

}
void HttpResponse::make_response_nocgi(struct evbuffer *out)
{
	make_response_line(out);
	make_response_head(out);
	add_response_file(out);
}

void HttpResponse::make_response_cgi(struct evbuffer* out)
{
	//std::cout << "cgi" << std::endl;
	FCGI m_fcgi;
	m_fcgi.start_connect(); //连接php-fpm
	m_fcgi.send_begin_request_record();
	m_fcgi.send_params_record("SCRIPT_FILENAME", m_http_request->get_http_path());  //脚本文件路径
	std::cout << "php file path: " << m_http_request->get_http_path() << std::endl;
	std::cout << "php file param: " << m_http_request->get_http_param() << std::endl;

	if (m_http_request->get_http_method() == "GET")
	{
		m_fcgi.send_params_record("REQUEST_METHOD", "GET");
		m_fcgi.send_params_record("QUERY_STRING", m_http_request->get_http_param());
		//...其他的name-value对;
		m_fcgi.send_empty_params_record(); //发送空的name-value表明发送name-value结束
	}
	else
	{
		//post,还要发送stdin
		m_fcgi.send_params_record("REQUEST_METHOD", "POST");

		//如果url中有?
		if (m_http_request->get_http_param().empty() == false)
		{
			m_fcgi.send_params_record("QUERY_STRING", m_http_request->get_http_param());
		}
        m_fcgi.send_params_record("CONTENT_LENGTH", std::to_string(m_http_request->get_http_content_length()));
        m_fcgi.send_params_record("CONTENT_TYPE", "application/x-www-form-urlencoded");
		//...其他的name-value对;
		m_fcgi.send_empty_params_record(); //发送空的name-value表明发送name-value结束

		m_fcgi.send_stdin_record(m_http_request->get_http_recv_evb()); //发送post的content
		m_fcgi.send_empty_stdin_record();
	}

	//将内容读到m_send_evb;以下内容可以作为回调?
	if(m_fcgi.read_from_php_fpm(m_send_evb) == false)
	{
		//NOT FOUND
		m_code = 404;
		//移除evbuffer数据
		evbuffer_drain(m_send_evb, evbuffer_get_length(m_send_evb));
		/* line */
		make_response_line(out);
		char *_code_str = const_cast<char*>(http_response_phrase_internal(m_code));
		evbuffer_add_printf(m_send_evb,
                    "<HTML><HEAD>\n"
	    			"<TITLE>%d %s</TITLE>\n"
	    			"</HEAD><BODY>\n"
	    			"<H1>%s</H1>\n"
	    			"</BODY></HTML>\n",
		    		m_code,
		   			_code_str,
		    		_code_str);

		m_content_length = evbuffer_get_length(m_send_evb);
		/* head */
		evbuffer_add_printf(out, "Content-Type: text/html\n");
		evbuffer_add_printf(out, "Content-Length: %d\n", m_content_length);
		evbuffer_add_printf(out, "\n");
		/* content */
		add_response_file(out);

	}
	else
	{
		//构建回应报文
		//处理一下evbuffer的内容
		make_response_line(out);

		size_t len;
		char *readln;
		while(1)
		{
			readln = evbuffer_readln(m_send_evb, &len, EVBUFFER_EOL_CRLF);
			//因为evbuffer_readln读取"\n"返回NULL
			if(strlen(readln) == 0)
			{
				free(readln); //忘写导致内存泄漏;因为evbuffer_readln使用了malloc
				break;
			}

			evbuffer_add_printf(out, "%s\n", readln);
			free(readln); //evbuffer_readln使用了malloc
		}

		m_content_length = evbuffer_get_length(m_send_evb);

		evbuffer_add_printf(out, "Content-Length: %d\n\n", m_content_length);  //结束回应head
		
		add_response_file(out);
	}
}