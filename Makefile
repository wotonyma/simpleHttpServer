#common makefile header
CC = g++
TARGET = main
SOURCE = main.cpp ConnectionSocket.cpp Listener.cpp Threadpool.cpp HttpRequest.cpp HttpResponse.cpp http.cpp FCGI.cpp
CFLAGS = -g -Wall
LDFLAGS := -lpthread -levent -std=c++11

$(TARGET): $(SOURCE)
	$(CC) -o $(TARGET) $(SOURCE) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.o

.PHONY:clean
