//http.cpp

#include "http.h"

const std::string WEB_ROOT = "/home/huqiang/httpdTest";   //使用绝对地址
const std::string HOME_PAGE = "index.html";

//请求后缀的映射
std::unordered_map<std::string, std::string> stuffix_map{
	{".asp", "text/asp"},
	{".avi", "video/avi"},
	{".apk", "application/vnd.android.package-archive"},
	{".biz", "text/xml"},
	{".bmp", "application/x-bmp"},
	{".cml", "text/xml"},
	{".css", "text/css"},
	{".dll", "application/x-msdownload"},
	{".doc", "application/msword"},
	{".exe", "application/x-msdownload"},
	{".gif", "image/gif"},
    {".html","text/html"},
    {".htm", "text/html"},
    {".htx", "text/html"},
    {".ico", "image/x-icon"},
    {".img", "application/x-img"},
    {".IVF", "video/x-ivf"},
    {".java", "java/*"},
    {".jpe", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".jpg", "image/jpeg"},
    {".js", "application/x-javascript"},
    {".json", "application/json"},
    {".mp3", "audio/mp3"},
    {".mp4", "video/mpeg4"},
    {".png", "image/png"},
    {".torrent", "application/x-bittorrent"},
    {".wav", "audio/wav"},
    {".wmv", "video/x-ms-wmv"},
    {".xls", "application/vnd.ms-excel"},
    {".xml", "text/xml"},
    {".woff", "application/x-font-woff"},
    {".woff2", "application/x-font-woff2"},
    {".ttf", "application/x-font-truetype"},
    {".svg", "image/svg+xml"},
    {".otf", "application/x-font-opentype"},
    {".eot", "application/vnd.ms-fontobject"}
};


const char *informational_phrases[] = {
	/* 100 */ "Continue",
	/* 101 */ "Switching Protocols"
};

const char *success_phrases[] = {
	/* 200 */ "OK",
	/* 201 */ "Created",
	/* 202 */ "Accepted",
	/* 203 */ "Non-Authoritative Information",
	/* 204 */ "No Content",
	/* 205 */ "Reset Content",
	/* 206 */ "Partial Content"
};

const char *redirection_phrases[] = {
	/* 300 */ "Multiple Choices",
	/* 301 */ "Moved Permanently",
	/* 302 */ "Found",
	/* 303 */ "See Other",
	/* 304 */ "Not Modified",
	/* 305 */ "Use Proxy",
	/* 307 */ "Temporary Redirect"
};

const char *client_error_phrases[] = {
	/* 400 */ "Bad Request",
	/* 401 */ "Unauthorized",
	/* 402 */ "Payment Required",
	/* 403 */ "Forbidden",
	/* 404 */ "Not Found",
	/* 405 */ "Method Not Allowed",
	/* 406 */ "Not Acceptable",
	/* 407 */ "Proxy Authentication Required",
	/* 408 */ "Request Time-out",
	/* 409 */ "Conflict",
	/* 410 */ "Gone",
	/* 411 */ "Length Required",
	/* 412 */ "Precondition Failed",
	/* 413 */ "Request Entity Too Large",
	/* 414 */ "Request-URI Too Large",
	/* 415 */ "Unsupported Media Type",
	/* 416 */ "Requested range not satisfiable",
	/* 417 */ "Expectation Failed"
};

const char *server_error_phrases[] = {
	/* 500 */ "Internal Server Error",
	/* 501 */ "Not Implemented",
	/* 502 */ "Bad Gateway",
	/* 503 */ "Service Unavailable",
	/* 504 */ "Gateway Time-out",
	/* 505 */ "HTTP Version not supported"
};


#ifndef MEMBERSOF
#define MEMBERSOF(x) (sizeof(x)/sizeof(x[0]))
#endif

//第二个数据用来存放此类code的个数
const struct response_class response_classes[] = {
	/* 1xx */ { "Informational", MEMBERSOF(informational_phrases), informational_phrases },
	/* 2xx */ { "Success", MEMBERSOF(success_phrases), success_phrases },
	/* 3xx */ { "Redirection", MEMBERSOF(redirection_phrases), redirection_phrases },
	/* 4xx */ { "Client Error", MEMBERSOF(client_error_phrases), client_error_phrases },
	/* 5xx */ { "Server Error", MEMBERSOF(server_error_phrases), server_error_phrases }
};

const char *http_response_phrase_internal(int code)
{
	int klass = code / 100 - 1;
	int subcode = code % 100;

	/* Unknown class - can't do any better here */
	if (klass < 0 || klass >= (int) MEMBERSOF(response_classes))
		return "Unknown Status Class";

	/* Unknown sub-code, return class name at least */
	if (subcode >= (int) response_classes[klass].num_responses)
		return response_classes[klass].name;

	return response_classes[klass].responses[subcode];
}

//错误处理函数...




