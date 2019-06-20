//Listener.cpp

#include "Listener.h"

Listener::Listener()
{
	//...
	backlog = 10;
	pool = NULL;
}
Listener::Listener(string ip, short port)
{
	//初始化建立连接
	this->serverip = ip;
	this->port = port;
	backlog = 10;
	pool = NULL;
}

Listener::~Listener()
{
	event_base_loopexit(base, 0);
	evconnlistener_free(listener);
	event_base_free(base);
}

void Listener::set_server_ip(string ip)
{
	this->serverip = ip;
}

void Listener::set_port(short port)
{
	this->port = port;
}

void Listener::set_backlog(int backlog)
{
	this->backlog = backlog;
}

void Listener::set_pool(Threadpool *p)
{
	this->pool = p;
}

evutil_socket_t Listener::get_listener_fd() const
{
	return listener_fd;
}

int Listener::get_backlog() const
{
	return backlog;
}

string Listener::get_server_ip() const
{
	return serverip;
}

short Listener::get_port() const
{
	return port;
}

struct evconnlistener * Listener::get_evconnlistener() const
{
	return listener;
}

Threadpool *Listener::get_pool() const
{
	return pool;
}

int Listener::start_to_listen()
{
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, serverip.data(), &serveraddr.sin_addr);
	serveraddr.sin_port = htons(port);

	listener_fd = socket(AF_INET, SOCK_STREAM, 0);
	bind(listener_fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

	base = event_base_new();
	if(!base)
	{
		fprintf(stderr, "new base error\n");
		return -1;
	}

	listener = evconnlistener_new(base, do_accept, pool, 
		LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
		backlog, listener_fd);
	if(!listener)
	{
		fprintf(stderr, "new evconnlistener error:\n");
		return -1;
	}

	//设置accept函数出错处理
	evconnlistener_set_error_cb(listener, accept_error);

	//进入事件循环,监听端口
	event_base_dispatch(base);

	return 0;
}

int Listener::stop_listening()
{
	int result = evconnlistener_disable(NULL);
	return result; 
}

int Listener::restart_to_listen()
{
	int result = evconnlistener_enable(NULL);
	return result;
}

void Listener::do_accept(struct evconnlistener *listener, evutil_socket_t sockfd, 
		struct sockaddr *clentaddr, int len, void *data)
{
	//sockfd是新的连接fd,pool指针不能直接使用,传进去
	Threadpool * mypool = (Threadpool *) data;
	//Random随机法选择一个线程的管道发送数据;
	srand(sockfd);
	int num = rand() % mypool->get_current_num();
	//随机数[0,num-1]

	WorkerInfo *myworkers = mypool->get_workers_info();
	int sendfd = myworkers[num].notify_write_fd;
	write(sendfd, &sockfd, sizeof(evutil_socket_t));
	
	printf("send sockfd to thread tid = %u\n", myworkers[num].tid);
	//send(sockfd, &len, sizeof(len), 0);
}

void Listener::accept_error(struct evconnlistener *listener, void *arg)
{
	//获取event_base
	struct event_base *b = evconnlistener_get_base(listener);
	//获取错误字符
	int err = EVUTIL_SOCKET_ERROR();

	fprintf(stderr,"Get Error %d (%s)\n",err,evutil_socket_error_to_string(err));
	//shotdown这个端口
	event_base_loopexit(b, NULL);
}


//////////////////////////////////////////////////////////////////////////////////
/* 自定义的信号处理函数 */
//std::function<void(int, short,void *)> func
bool Listener::add_signal_event(int sig, void (*ptr)(int, short, void*))
{
	//新建一个信号事件注册回调函数
	/*listener.add_signal_event(SIGUSR1,bind<&Listener::event_timercb_xx, this, 
			std::placeholders::_1,std::placeholders::_2,std::placeholders::_3>)*/
	struct event *ev = evsignal_new(base, sig, ptr, (void*)this);
	//是否创建成功和使生效成功
	if ( !ev || event_add(ev, NULL) < 0 )
	{
		//使事件失效
		event_del(ev);
		event_free(ev);
		return false;
	}
	//删除旧的信号事件（同一个信号只能有一个信号事件）
	delete_signal_event(sig);
	signal_events[sig] = ev;
	return true;
}

/* 删除自定义的信号 */
bool Listener::delete_signal_event(int sig)
{
	map<int, event*>::iterator iter = signal_events.find(sig);
	if( iter == signal_events.end() )return false;
	event_del(iter->second);
	event_free(iter->second);
	signal_events.erase(iter);
	return true;
}

//添加定时器事件
/*std::function<void(int, short, void *)> ptr = std::bind<&Listener::event_timercb_xx, this, 
			std::placeholders::_1,std::placeholders::_2,std::placeholders::_3>;*/
event *Listener::add_timer_event(void (*ptr)(int, short, void *), timeval tv, bool once)
{	
	int flag = 0;

	//是否永久触发
	if( !once )
		flag = EV_PERSIST;
	//新建定时器信号事件
	struct event *ev = event_new(base, -1, flag, ptr, (void *)this);
	if( event_add(ev, &tv) < 0 )
	{
		event_del(ev);
		event_free(ev);
		return NULL;
	}
	return ev;
}


//删除定时器事件
bool Listener::delete_timer_event(struct event *ev)
{
	int res = event_del(ev);
	event_free(ev);
	return (0 == res);
}
