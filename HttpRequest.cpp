//HttpRequest.cpp

#include "HttpRequest.h"

HttpRequest::HttpRequest()
{
    m_cgi = false;
    m_path = WEB_ROOT;
    m_resource_size = 0;
    m_content_length = -1;
    m_resource_suffix = ".html";

    memset(&m_file_stat, 0, sizeof(struct stat));

    m_recv_evb = evbuffer_new();

}


HttpRequest::~HttpRequest()
{
	//反初始化释放堆上分配的空间
	evbuffer_free(m_recv_evb);
}

void HttpRequest::set_http_method(string _method)
{

	char *c_method = const_cast<char*>(_method.c_str());

	if(strcasecmp(c_method, "GET") == 0)
		m_method = HTTP_REQ_GET;

	else if(strcasecmp(c_method, "POST") == 0)
		m_method = HTTP_REQ_POST;

	else if(strcasecmp(c_method, "HEAD") == 0)
		m_method = HTTP_REQ_HEAD;

	else if(strcasecmp(c_method, "PUT") == 0)
		m_method = HTTP_REQ_PUT;

	else if(strcasecmp(c_method, "DELETE") == 0)
		m_method = HTTP_REQ_DELETE;

	else if(strcasecmp(c_method, "OPTIONS") == 0)
		m_method = HTTP_REQ_OPTIONS;

	else if(strcasecmp(c_method, "TRACE") == 0)
		m_method = HTTP_REQ_TRACE;

	else if(strcasecmp(c_method, "CONNECT") == 0)
		m_method = HTTP_REQ_CONNECT;

	else if(strcasecmp(c_method, "PATCH") == 0)
		m_method = HTTP_REQ_PATCH;
	
	else
		m_method = static_cast<enum http_request_method>(0);
}

const string HttpRequest::get_http_method() const
{
	string method;

	switch (m_method) {
	case HTTP_REQ_GET:
		method = "GET";
		break;
	case HTTP_REQ_POST:
		method = "POST";
		break;
	case HTTP_REQ_HEAD:
		method = "HEAD";
		break;
	case HTTP_REQ_PUT:
		method = "PUT";
		break;
	case HTTP_REQ_DELETE:
		method = "DELETE";
		break;
	case HTTP_REQ_OPTIONS:
		method = "OPTIONS";
		break;
	case HTTP_REQ_TRACE:
		method = "TRACE";
		break;
	case HTTP_REQ_CONNECT:
		method = "CONNECT";
		break;
	case HTTP_REQ_PATCH:
		method = "PATCH";
		break;
	default:
		/* 不赋值,为空 */
		break;
	}

	return (method);
}

void HttpRequest::set_http_url(const string& _url)
{
	m_url = _url;
}

string& HttpRequest::get_http_url()
{
	return m_url;
}

void HttpRequest::set_http_version(const string& _version)
{
	m_version = _version;
}

const string& HttpRequest::get_http_version() const
{
	return m_version;
}


//解析请求行
bool HttpRequest::request_line_parse(struct evbuffer *evb)
{
	size_t len;
	char * evb_rdln = evbuffer_readln(evb, &len, EVBUFFER_EOL_CRLF);
	//因为evbuffer_readln读取"\n"会返回NULL;
	if (strlen(evb_rdln) == 0)
	{
		cout << "request line is null" << std::endl;
		return false;
	}

	std::cout << evb_rdln << endl;

	std::stringstream ss(evb_rdln);  //创建string流,用一行初始化

	string _method;
	string _url;
	string _version;
	ss >> _method >> _url >> _version;  //获得每个字段

	this->set_http_method(_method);
	this->set_http_url(_url);
	this->set_http_version(_version);

	free(evb_rdln);

	return true;
}

//判断方法是否解析成功
bool HttpRequest::is_method_ok()
{
	string _method = this->get_http_method();
	if (_method.empty() == true)
	{
		m_code = 405;
	}
	if(strcasecmp(_method.c_str(), "GET") == 0)
		return true;
	else if (strcasecmp(_method.c_str(),"POST") == 0)
	{
		m_cgi = true;
		return true;
	}
	//else if 其他方法,目前只支持getpost
	else
	{
		m_code = 405;
		return false;
	}
	return true;
}

//解析url,get方法可能带参数
void HttpRequest::url_parse()
{
	if(m_method == HTTP_REQ_GET)
	{
		std::cout << "get HttpRequest::url_parse() " << m_url << std::endl;
		std::size_t pos = m_url.find('?');
		if (pos != std::string::npos)
		{
			//带参数get
			//m_cgi = true;代码错误逻辑,比如woff?v=4.7.0
			//默认路径下的文件;substr(start, len),len = (pos - 1) + 1
			m_path += m_url.substr(0, pos);
			std::cout << "get HttpRequest::url_parse() " << m_path << std::endl;

			//获取参数pos+1到字符串结束
			m_param += m_url.substr(pos + 1);
			std::cout << "get HttpRequest::url_parse() " << m_param << std::endl;
		}
		else
		{
			//不是带参数get
			m_path += m_url;
		}
	}
	else if (m_method == HTTP_REQ_POST)
	{
		std::cout << "post HttpRequest::url_parse() " << m_url << std::endl;
		std::size_t pos = m_url.find('?');
		if (pos != std::string::npos)
		{
			//带?的post
			m_path += m_url.substr(0, pos);
			std::cout << "post HttpRequest::url_parse() " << m_path << std::endl;

			//获取参数pos+1到字符串结束
			m_param += m_url.substr(pos + 1);
			std::cout << "post HttpRequest::url_parse() " << m_param << std::endl;
		}
		else
		{
			//不带?的post
			m_path += m_url;
		}
	}
	//else if...其他方法

	/*判断是否是请求的文件
	  如果是一个目录则返回默认页面*/
	if (m_path[m_path.length() - 1] == '/')
	{
		m_path += HOME_PAGE;
	}
}

//判断路径的文件是否存在
int HttpRequest::file_stat_parse()
{
	int code;
	int res = stat(m_path.c_str(), &m_file_stat);
	if (res < 0)
	{
		/* 根据返回值的error返回不同的code,目前统一返回404,可扩展 */
		std::cout << "Path Not Found" << std::endl;
		code = 404;
		return code;
	}

	/*判断是否是目录,是目录可以禁止访问,也可以跳转到默认页面,也可以显示目录*/
	if (S_ISDIR(m_file_stat.st_mode))
	{
		/* 默认页面 */
		/*m_path += "/";
		m_path += HOME_PAGE;
		if(stat(m_path.c_str(), &m_file_stat) < 0)
		{
			code = 404
			return code;
		}*/

		/* 禁止访问 */
		code = 403;
		return code;
	}
	else
    {	//很多文件具有owner执行权限,却没有群组执行权限,比如一张图片
        /*if((m_file_stat.st_mode & S_IXUSR ) || (m_file_stat.st_mode & S_IXGRP) || (m_file_stat.st_mode & S_IXOTH))
        {
           m_cgi = true;
        }*/
        /* 如果是可执行文件 */
        if((m_file_stat.st_mode & S_IXGRP) || (m_file_stat.st_mode & S_IXOTH))
        {
           m_cgi = true;
        }
    }

    /* 请求文件大小 */
    m_resource_size = m_file_stat.st_size;
    /* 获取后缀 */
    std::size_t pos = m_path.rfind('.'); //反向查找
    if (pos != std::string::npos)
    {
    	m_resource_suffix = m_path.substr(pos);
    }
    
    code = 200;
    return code;
}

void HttpRequest::request_head_parse(struct evbuffer *evb)
{
	size_t len;
	char *readln;
	std::string str;
	while(1)
	{
		readln = evbuffer_readln(evb, &len, EVBUFFER_EOL_CRLF);
		//因为evbuffer_readln读取"\n"返回NULL
		if(strlen(readln) == 0)
		{
			free(readln); //忘写导致内存泄漏;因为evbuffer_readln使用了malloc
			break;
		}

		//对一行进行解析
		else
		{
			str = readln;
			std::size_t pos = str.find(": ");
			if (pos != std::string::npos)
			{
				string key = str.substr(0, pos);
				//剩下的字符包括结尾的空字符和:_ (_代表空格)
				string value = str.substr(pos + 2, str.size() - 3 - pos);

				//将key-value加入key-value map
				m_head_kvmap.insert(make_pair(key, value));
			}
		}
		free(readln); //evbuffer_readln使用了malloc
	}
}

//是否使用了recv_evb
bool HttpRequest::is_need_recv_text()
{
	//目前只实现get/post
	if(m_method == HTTP_REQ_POST)
		return true;
	else
		return false;
}

//是否是cgi程序
bool HttpRequest::is_cgi()
{
	return m_cgi;
}

void HttpRequest::set_http_path(string _path)
{
	m_path = _path;
}

string & HttpRequest::get_http_path()
{
	return m_path;
}

void HttpRequest::set_http_param(string _param)
{
	m_param = _param;
}
string & HttpRequest::get_http_param()
{
	return m_param;
}

void HttpRequest::set_http_resource_size(int _resource_size)
{
	m_resource_size = _resource_size;
}

int HttpRequest::get_http_resource_size()
{
	return m_resource_size;
}

void HttpRequest::set_http_resource_suffix(string _resource_suffix)
{
	m_resource_suffix = _resource_suffix;
}

string & HttpRequest::get_http_resource_suffix()
{
	return m_resource_suffix;
}

//request实体长度,用于接收实体
void HttpRequest::set_http_content_length(int _content_length)
{
	m_content_length = _content_length;
}

//根据kymap设置长度
void HttpRequest::set_http_content_length_on_kvmap()
{
	//有返回value,没有插入V()返回,对string来说是空
	string value = m_head_kvmap["Content-Length"];
	if( ! value.empty())
	{
		/*sscanf(value.c_str(), "%d", &m_content_length)*/
		stringstream ss(value);
		ss >> m_content_length;
	}
}

int HttpRequest::get_http_content_length()
{
	return m_content_length;
}

//返回unordered_map的引用
std::unordered_map<std::string, std::string> & HttpRequest::get_http_head_kvmap()
{
	return m_head_kvmap;
}


//将剩余读缓冲区的数据读取到缓冲区
int HttpRequest::get_http_recv_data(struct evbuffer* evb)
{
	//content-length error:原因是因为没有Content-Length会出现错误
	//int len;
	//if(m_content_length > 0)  //有Content-Length
	//{
		//len = evbuffer_remove_buffer(evb, m_recv_evb, m_content_length);
	//if (len != m_content_length) 错误处理:

	//如果是post方法,参数是主体
		/*if(m_method == HTTP_REQ_POST)
		{
			char *data;
			evbuffer_copyout(m_recv_evb, data, len);
			m_param = data;
		}*/

	//}
	//else  //没有content-Length
	//{
		
	//}
        evbuffer_add_buffer(m_recv_evb, evb);
        m_content_length = evbuffer_get_length(m_recv_evb);
	return 0;
}

//获取缓冲区指针
struct evbuffer* HttpRequest::get_http_recv_evb()
{
	return m_recv_evb;
}

void HttpRequest::set_code(int _code)
{
	m_code = _code;
}

int HttpRequest::get_code()
{
	return m_code;
}


void HttpRequest::all_request_parse(struct evbuffer* evb)
{
	//进行每一步解析并赋值code
	if(request_line_parse(evb) == false)
	{
		m_code = 400;
		return;
	}

	if (is_method_ok() == false)
	{
		m_code = 405;//不支持的方法
	}

	url_parse();

	m_code = file_stat_parse();

	request_head_parse(evb);

	if(is_need_recv_text())
	{
		set_http_content_length_on_kvmap();
		get_http_recv_data(evb);
	}

	//测试用
	std::cout << get_http_method() << std::endl;
	std::cout << m_path << std::endl;
	std::cout << m_code << std::endl;

}

