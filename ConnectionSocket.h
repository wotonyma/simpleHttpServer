//ConnectionSocket.h
#ifndef CONNECTIONSOCKET_H_
#define CONNECTIONSOCKET_H_

#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

#include "WorkerInfo.h"

class ConnectionSocket
{
public:
	ConnectionSocket();
	~ConnectionSocket();

	void setconnfd(evutil_socket_t fd);
	void setbev(struct bufferevent *buf);
	void setinputbuf(struct evbuffer *in);
	void setoutputbuf(struct evbuffer *out);
	void setworker(struct WorkerInfo *wk);

	evutil_socket_t getconnfd();
	struct bufferevent* getbev();
	struct evbuffer* getinput();
	struct evbuffer* getoutput();
	struct WorkerInfo* getworker();

private:
	evutil_socket_t sockfd;
	struct bufferevent *bev;
	struct evbuffer *input;
	struct evbuffer *output;
	struct WorkerInfo *m_worker;

};

#endif
