CC = gcc
CFLAGS = -Wall -Wextra -fPIC

LIBRARY = libreactor.so
OBJECTS = reactor.o threadpool.o

all: react_server

react_server: react_server.o $(LIBRARY)
	$(CC) $(CFLAGS) react_server.o -L. -Wl,-rpath=. -lreactor -pthread -o react_server


react_server.o: react_server.c
	$(CC) $(CFLAGS) -c react_server.c -o react_server.o

$(LIBRARY): $(OBJECTS)
	$(CC) $(CFLAGS) -shared -o $(LIBRARY) $(OBJECTS)

reactor.o: reactor.c reactor.h
	$(CC) $(CFLAGS) -c reactor.c -o reactor.o

threadpool.o: threadpool.c threadpool.h
	$(CC) $(CFLAGS) -c threadpool.c -o threadpool.o

clean:
	rm -f react_server $(LIBRARY) $(OBJECTS) react_server.o
