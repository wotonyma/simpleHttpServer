//Threadpool.h

#ifndef THREADPOOL_H_
#define THREADPOOL_H_


#include "WorkerInfo.h"
#include "ConnectionSocket.h"

#include "HttpRequest.h"
#include "HttpResponse.h"

#include <map>
using std::map;
#include <algorithm>

#include <unistd.h>
#include <stdio.h>

/*线程池*/
class Threadpool
{
public:
	Threadpool();
	Threadpool(int max, int current);
	~Threadpool(); 
	void setup_thread(WorkerInfo *worker);  //初始化设置每个线程的信息
	static void *worker_thread(void *arg);	 //线程的入口函数

	static void event_readcb(struct bufferevent *bev, void *data);	//连接读缓冲区回调函数
	static void event_writecb(struct bufferevent *bev, void *data); //连接写缓冲区回调函数
	static void event_errorcb(struct bufferevent *bev, short events, void *data); //连接其他事件回调函数

	static void notify_readcb(evutil_socket_t notify_read_fd, short events, void *data);   //管道读回调事件

	void start_run(); //开启线程池
	void stop_run(timeval *tv);

public:
	int get_current_num();
	WorkerInfo *get_workers_info();

	static void error_quit(const char *str);  //错误处理函数


private:
	int max_thread_num; //线程池最大数量
	int current_num;  //线程池当前数量
	WorkerInfo *workers;  //线程信息数组的指针

public:
	static const int EXIT_CODE = -1; //主线程结束子线程的退出码

	//friend class Listener;
	
};

#endif


