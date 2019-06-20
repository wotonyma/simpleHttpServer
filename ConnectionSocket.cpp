//ConnectionSocket.cpp

#include "ConnectionSocket.h"

ConnectionSocket::ConnectionSocket()
{
	//初始化
}

ConnectionSocket::~ConnectionSocket()
{
	//反初始化
}

//设置连接的文件描述符
void ConnectionSocket::setconnfd(evutil_socket_t fd)
{
	sockfd = fd;
}

//设置连接的缓冲区的指针
void ConnectionSocket::setbev(struct bufferevent *buf)
{
	bev = buf;
}

//设置读缓冲区的指针
void ConnectionSocket::setinputbuf(struct evbuffer *in)
{
	input = in;
}

//设置写缓冲区的指针
void ConnectionSocket::setoutputbuf(struct evbuffer *out)
{
	output = out;
}

//设置从属的工作线程
void ConnectionSocket::setworker(struct WorkerInfo *wk)
{
	m_worker = wk;
}

//获取连接的文件描述符
evutil_socket_t ConnectionSocket::getconnfd()
{
	return sockfd;
}

//获取缓冲区指针
struct bufferevent* ConnectionSocket::getbev()
{
	return bev;
}

//获取读缓冲区指针
struct evbuffer* ConnectionSocket::getinput()
{
	return input;
}

//获取写缓冲区指针
struct evbuffer* ConnectionSocket::getoutput()
{
	return output;
}

//获取从属线程信息
struct WorkerInfo* ConnectionSocket::getworker()
{
	return m_worker;
}