//Threadpool.cpp

#include "Threadpool.h"

Threadpool::Threadpool()
{
	max_thread_num = 10; //默认线程最大数为10
	//获取系统cpu核数;
	long num_procs = sysconf(_SC_NPROCESSORS_CONF);
	current_num = (int)num_procs * 2; //默认开启子线程数
	workers = new WorkerInfo[max_thread_num];	//创建线程信息的数组
	//设置每一个线程的信息
	for (int i = 0; i < current_num; ++i)
	{
		setup_thread(&workers[i]);
	}
}

Threadpool::Threadpool(int max, int current)
{
	//初始化各项数据
	max_thread_num = max;
	current_num = current;
	workers = new WorkerInfo[max_thread_num];	//创建线程信息的数组

	//设置每一个线程的信息
	for (int i = 0; i < current_num; ++i)
	{
		setup_thread(&workers[i]);
	}
}

Threadpool::~Threadpool()
{
	//停止事件循环（如果事件循环没开始，则没效果）
	stop_run(NULL);
 
	//释放各个子线程的资源
	for(int i=0; i<current_num; i++)
	{
		close(workers[i].notify_read_fd);
		close(workers[i].notify_write_fd);
		event_free(workers[i].notify_event);
		event_base_free(workers[i].base);
	}
	delete [] workers;
}

//初始化base,管道
void Threadpool::setup_thread(WorkerInfo *worker)
{
	//创建根基事件
	worker->base = event_base_new();
	if (worker->base == NULL)
	{
		error_quit("event base new error");
	}

	//创建管道
	int fds[2];
	if(-1 == pipe(fds))
	{
		error_quit("create pipe error");
	}
	worker->notify_read_fd = fds[0];
	worker->notify_write_fd = fds[1];

	/*event_new(struct event_base *base, evutil_socket_t fd, short events, 
		void (*cb)(evutil_socket_t, short, void *), void *arg)*/
	//创建管道读事件,监听管道,传入worker
	worker->notify_event = event_new(worker->base, worker->notify_read_fd, 
		EV_READ | EV_PERSIST, notify_readcb, worker);
	event_base_set(worker->base, worker->notify_event);
	if (event_add(worker->notify_event, 0) == -1)
	{
		error_quit("notify pipe event add error");
	}

	printf("setup threads\n");
}

void *Threadpool::worker_thread(void *arg)
{
	WorkerInfo *worker = (WorkerInfo *)arg;
	//开启libevent的事件循环，准备处理业务
	printf("thread %u started\n", (unsigned int)worker->tid);
	event_base_dispatch(worker->base);

	printf("subthread done\n");
	pthread_exit(0);
}

//管道读的回调函数
void Threadpool::notify_readcb(evutil_socket_t notify_read_fd, short events, void *data)
{
	printf("notify_readcb\n");
	WorkerInfo *worker = (WorkerInfo *)data;

	//用于保存新的连接的描述符
	evutil_socket_t fd;

	//从管道读取数据
	if(read(notify_read_fd, &fd, sizeof(evutil_socket_t)) <= 0)
		error_quit("read pipe error");

	//如果是退出码,终止事件循环
	if(EXIT_CODE == notify_read_fd)
	{
		event_base_loopexit(worker->base, NULL);
		printf("event base loopexit\n");
		return;
	}

	//新的连接
	struct bufferevent *bev;
	//BEV_OPT_CLOSE_ON_FREE释放buffer后释放底层socket连接
	bev = bufferevent_socket_new(worker->base, fd, BEV_OPT_CLOSE_ON_FREE);
	if(NULL == bev)
	{
		error_quit("bufferevent new error");
		event_base_loopexit(worker->base, NULL);
		return;
	}
 
	//设置ConnectionSocket
	ConnectionSocket conn;
	conn.setworker(worker);
	conn.setconnfd(fd);
	conn.setbev(bev);
	struct evbuffer* in = bufferevent_get_input(bev);
	struct evbuffer* out = bufferevent_get_output(bev);
	conn.setinputbuf(in);
	conn.setoutputbuf(out);
	//将该链接放入队列
	worker->ConnQueue.push_back(conn);

	//设置读写错误的回调函数
	//worker->ConnQueue.back()得到刚插入队列连接的引用
	bufferevent_setcb(bev, event_readcb, event_writecb, event_errorcb, 
		(void *)&(worker->ConnQueue.back()));
	bufferevent_enable(bev, EV_WRITE);
	bufferevent_enable(bev, EV_READ);

	//调用用户自定义的连接事件处理函数
	//ConnectionEvent(conn);
}

void Threadpool::start_run()
{
	//创建线程
	int result;
	for (int i = 0; i < current_num; ++i)
	{
		result = pthread_create(&workers[i].tid, NULL, worker_thread, (void *)&workers[i]);
		if(-1 == result)
			error_quit("thread create error");
	}
	printf("create threads\n");

}

void Threadpool::stop_run(timeval *tv)
{
	int contant = EXIT_CODE;
	//向各个子线程的管道中写入EXIT_CODE，通知它们退出
	for(int i=0; i<current_num; i++)
	{
		write(workers[i].notify_write_fd, &contant, sizeof(int));
	}
}

int Threadpool::get_current_num()
{
	return current_num;
}

WorkerInfo *Threadpool::get_workers_info()
{
	return workers;
}

void Threadpool::error_quit(const char *str)
{
	//输出错误信息，退出程序
	fprintf(stderr, "%s", str);
	if( errno != 0 ) 
		fprintf(stderr, " : %s", strerror(errno)); 
	fprintf(stderr, "\n");    
	exit(1);
}

//bufferevent读回调
void Threadpool::event_readcb(struct bufferevent *bev, void *data)
{
	printf("ReadEventCb\n");
	ConnectionSocket *conn = (ConnectionSocket *)data;

	struct evbuffer *input = bufferevent_get_input(bev);
	struct evbuffer *output = bufferevent_get_output(bev);

	HttpRequest *m_request = new HttpRequest();
	m_request->all_request_parse(input);
	HttpResponse *m_response = new HttpResponse(m_request);
	m_response->make_response(output);

	delete m_response;
	delete m_request;
}

//bufferevent写回调

void Threadpool::event_writecb(struct bufferevent *bev, void *data)
{

	//printf("WriteEventcb\n");
	//ConnectionSocket *conn = (ConnectionSocket *)data;
}



//bufferevent错误关闭回调
void Threadpool::event_errorcb(struct bufferevent *bev, short events, void *data)
{

	//printf("CloseEventCb\n");
	//#define BEV_EVENT_READING   0x01    /**< 读取过程中遇到的错误 */
    //#define BEV_EVENT_WRITING   0x02    /**< 写数据过程中遇到错误 */
    //#define BEV_EVENT_EOF       0x10    /**< 读到了文件结束符EOF */
    //#define BEV_EVENT_ERROR     0x20    /**< 遇到了不可恢复的错误 */
    //#define BEV_EVENT_TIMEOUT   0x40    /**< 表示计时器的时间到了 */
    //#define BEV_EVENT_CONNECTED 0x80    /**< 表示connect操作完成 */

	ConnectionSocket *conn = (ConnectionSocket *)data;

	if(events & BEV_EVENT_READING)
	{
		//读取过程中遇到的错误;
		printf("reading error.\n");

		return;

	}


	if(events & BEV_EVENT_WRITING)
	{
		//写数据过程中遇到错误;
		printf("writing error.\n");

		return;
	}

	if(events & BEV_EVENT_EOF)
	{
		//读到了文件结束符EOF
		printf("file eof.\n");

		return;
	}

	if(events &  BEV_EVENT_ERROR)
	{
		//遇到了不可恢复的错误
		printf("unrecoverable error\n");
		int fd = conn->getconnfd();
		printf("fd = %d\n", fd);
		auto it = find_if(conn->getworker()->ConnQueue.begin(), 
			conn->getworker()->ConnQueue.end(),
			[=](ConnectionSocket c)->bool{
				return (c.getconnfd() == conn->getconnfd());
			});
		if(it != conn->getworker()->ConnQueue.end())
		{
			int fd1 = conn->getconnfd();
			printf("fd1 = %d\n", fd1);
			printf("eraser!!!!\n");
			conn->getworker()->ConnQueue.erase(it);
		}

		//释放缓冲区和套接字
		//bufferevent_free(conn->getbev());
		bufferevent_free(bev);
		
		if(close(fd) < 0)
		{
			printf("close fd error!!!\n");
		}

		return;
	}
	if(events & BEV_EVENT_TIMEOUT)
	{
		//表示计时器的时间到了
		printf("timeout.\n");
		int fd = conn->getconnfd();
		printf("fd = %d\n", fd);
		auto it = find_if(conn->getworker()->ConnQueue.begin(), 
			conn->getworker()->ConnQueue.end(),
			[=](ConnectionSocket c)->bool{
				return (c.getconnfd() == conn->getconnfd());
			});
		if(it != conn->getworker()->ConnQueue.end())
		{
			int fd1 = conn->getconnfd();
			printf("fd1 = %d\n", fd1);
			printf("eraser!!!!\n");
			conn->getworker()->ConnQueue.erase(it);
		}

		//释放缓冲区和套接字
		//bufferevent_free(conn->getbev());
		bufferevent_free(bev);
		
		if(close(fd) < 0)
		{
			printf("close fd error!!!\n");
		}

		return;
	}

	if(events & BEV_EVENT_CONNECTED)
	{
		//表示connect操作完成

		//从连接队列删除
		int fd = conn->getconnfd();
		printf("fd = %d\n", fd);
		auto it = find_if(conn->getworker()->ConnQueue.begin(), 
			conn->getworker()->ConnQueue.end(),
			[=](ConnectionSocket c)->bool{
				return (c.getconnfd() == conn->getconnfd());
			});
		if(it != conn->getworker()->ConnQueue.end())
		{
			int fd1 = conn->getconnfd();
			printf("fd1 = %d\n", fd1);
			printf("eraser!!!!\n");
			conn->getworker()->ConnQueue.erase(it);
		}

		//释放缓冲区和套接字
		//bufferevent_free(conn->getbev());
		bufferevent_free(bev);
		
		if(close(fd) < 0)
		{
			printf("close fd error!!!\n");
		}

		return;	

	}
	//从连接队列删除

    //其他错误

}
