//Listener.h
#ifndef LISTENER_H_
#define LISTENER_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
 
#include <map>
#include <string>
using std::map;

#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

#include <string>

#include "Threadpool.h"

using namespace std;
 
/*监听者*/
class Listener
{
public:
	Listener();
	Listener(string ip, short port);
	~Listener();

	int start_to_listen();
	int stop_listening();
	int restart_to_listen();

	static void do_accept(struct evconnlistener *listener, evutil_socket_t sockfd, 
		struct sockaddr *clientaddr, int len, void *data); //listener的回调函数,此处sockfd为客户端fd,data为线程池指针

	static void accept_error(struct evconnlistener *listener, void *arg);

	void set_server_ip(string ip);
	void set_port(short port);
	void set_backlog(int backlog);
	void set_pool(Threadpool *p);

	evutil_socket_t get_listener_fd() const;
	int get_backlog() const;
	string get_server_ip() const;
	short get_port() const;
	struct evconnlistener *get_evconnlistener() const;
	Threadpool *get_pool() const;

private:
	evutil_socket_t listener_fd;
	int backlog;  //监听的阻塞队列最大数
	string serverip;
	short port;
	struct sockaddr_in serveraddr; //服务器地址
	struct event_base *base;   //主线程根基事件
	struct evconnlistener *listener;  //主线程监听者,注册在base上

	Threadpool *pool;  //工作者线程池指针

	//管道通信

	//自定义的信号处理
	map<int, event*> signal_events; 

private:
	//禁止拷贝
	Listener(const Listener&);
	Listener& operator= (const Listener&);

public:
	//添加信号定时器处理
	bool add_signal_event(int sig, void (*ptr)(int, short, void*));
	bool delete_signal_event(int sig);
	event *add_timer_event(void (*ptr)(int, short, void *), timeval tv, bool once);
	bool delete_timer_event(struct event *ev);
	//自定义信号定时器处理函数
	//void event_signalcb_xx(int fd, short events, void *arg);
	//void event_timercb_xx(int fd, short events, void *arg);
	
};
#endif