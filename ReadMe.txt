#React Server
This is a simple React server that allows for handling an unlimited number of concurrent clients.

Prerequisites
C compiler


##Building the Server
Clone the repository:

git clone https://github.com/your-username/react-server.git

##Navigate to the project directory:

cd react-server

##Build the server:
make all

#Running the Server
##Start the server:

./react_serve

*The server will start listening on port 9034*

Testing the Server

To test the server's performance, you can use a tool like wrk.
Install wrk using the following command:

sudo apt-get install wrk
Send concurrent requests to the server:

wrk -t10 -c1000 -d30s http://localhost:9034/
(in my test i used port 12345 but you can use any port you want)
*This will send 10 threads, 1000 connections, for 30 seconds.

usage:
terminal 1:
 ./react_server

and type in terminal 2:
nc localhost 9034

and type in terminal 3:
telnet localhost 9034

*You can use Terminal 2 or 3 as a client to send messages to the server.


react_server.c:

The react_server.c file contains the main implementation of the React server.
It creates a server socket, listens for client connections, and handles client messages using the Reactor pattern.
The server can handle an unlimited number of concurrent clients.
It uses a thread pool to process client messages in separate threads for improved concurrency.
The processClientMessage function processes client messages and sends them to all connected clients except the sender.
The handleNewConnection function handles new client connections and adds the client socket to the Reactor.
The server is started and run using the Reactor.


reactor.c:

The createReactor function creates a new Reactor instance and initializes its members.
The stopReactor function stops the Reactor and cleans up its resources.
The reactorThread function is the entry point for the Reactor thread. It runs in a loop, waiting for events and calling the associated event handlers.
The startReactor function starts the Reactor and creates the Reactor thread.
The addFd function adds a file descriptor and its associated event handler to the Reactor for event monitoring.
The WaitFor function waits for the Reactor thread to finish.

threadpool:

The threadpool_create function creates a new ThreadPool instance and initializes its members.
It determines the number of available CPU cores and creates a worker thread for each core.
The worker function is the entry point for the worker threads. It waits for tasks in the task queue and executes them.
The threadpool_destroy function stops the thread pool and cleans up its resources.
The threadpool_add_work function adds a new task to the thread pool's task queue for execution.

