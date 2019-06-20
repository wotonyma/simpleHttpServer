//main.cpp
//linux库函数
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

///////////////////////////
#include "Listener.h"
#include "Threadpool.h"
#include "WorkerInfo.h"
#include "ConnectionSocket.h"

int main(int argc, char const *argv[])
{
	//evthread_use_pthreads();//开启多线程支持
	Threadpool tpool;
	tpool.start_run();

	/* 初始化listener */
	Listener httpListener;
	httpListener.set_server_ip("127.0.0.1");
	httpListener.set_port(8000);
	httpListener.set_backlog(10);
	httpListener.set_pool(&tpool);

	/* 开始监听 */
	httpListener.start_to_listen();
	
	return 0;
}
