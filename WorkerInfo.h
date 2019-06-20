//WorkerInfo.h

#ifndef WORKERINFO_H_
#define WORKERINFO_H_

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
 
#include <vector>
using std::vector;
 
#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

#include "ConnectionSocket.h"

//每个子线程的线程信息
class ConnectionSocket;

struct WorkerInfo
{
	pthread_t tid; //子线程id
	struct event_base *base;  //子线程的事件处理机
	struct event *notify_event; //子线程监听管道的事件机
	int notify_read_fd; //子线程管道的接收端
	int notify_write_fd; //子线程管道的发送端
	std::vector<ConnectionSocket> ConnQueue; //子线程的连接队列
	//Threadpool *pool; //保存线程池对象的指针
	//int fd[2];	
};

#endif
