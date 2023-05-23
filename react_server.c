#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <poll.h>
#include "reactor.h"

#define BUFFER_SIZE 1024
#define SERVER_PORT 9034

int serverSocket;
int* clientSockets;
int numClients = 0;
Reactor* reactor;

typedef struct {
    int clientSocket;
    char* buffer;
} ClientData;

void processClientMessage(void* data) {
    ClientData* clientData = (ClientData*)data;
    int clientSocket = clientData->clientSocket;
    char* buffer = clientData->buffer;

    ssize_t bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        printf("Received message from client: %s\n", buffer);

        // Send the message to all connected clients except the sending client
        for (int i = 0; i < numClients; i++) {
            if (clientSockets[i] != clientSocket) {
                ssize_t bytesSent = send(clientSockets[i], buffer, bytesRead, 0);
                if (bytesSent == -1) {
                    perror("Failed to send response to client");
                }
            }
        }
    } else if (bytesRead == 0) {
        for (int i = 0; i < numClients; i++) {
            if (clientSockets[i] == clientSocket) {
                clientSockets[i] = clientSockets[numClients - 1];
                numClients--;
                break;
            }
        }
        close(clientSocket);
    } else {
        perror("Failed to receive data from client");
    }

    free(buffer);
    free(clientData);
}

void handleClientMessage(int clientSocket) {
    ClientData* data = malloc(sizeof(ClientData));
    data->clientSocket = clientSocket;
    data->buffer = malloc(sizeof(char) * BUFFER_SIZE);

    threadpool_add_work(reactor->pool, processClientMessage, data);
}

void handleNewConnection() {
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);

    if (clientSocket == -1) {
        perror("Failed to accept client connection");
        return;
    }

    clientSockets = realloc(clientSockets, sizeof(int) * (numClients + 1));
    if (clientSockets == NULL) {
        perror("Failed to allocate memory for clientSockets");
        close(clientSocket);
        return;
    }

    addFd(reactor, clientSocket, handleClientMessage);
    clientSockets[numClients] = clientSocket;
    numClients++;
}
int main() {
    // Create thread pool
    ThreadPool* pool = threadpool_create(0);
    if (pool == NULL) {
        perror("Failed to create thread pool");
        return 1;
    }

    // Create server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Failed to create socket");
        return 1;
    }

    // Set up server address
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Bind the server socket to the specified address and port
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Failed to bind socket");
        close(serverSocket);
        return 1;
    }

    // Start listening for client connections
    if (listen(serverSocket, 5) == -1) {
        perror("Failed to listen for connections");
        close(serverSocket);
        return 1;
    }

    // Create a Reactor
    reactor = createReactor(pool);
    if (reactor == NULL) {
        perror("Failed to create Reactor");
        close(serverSocket);
        threadpool_destroy(pool); // Destroy the thread pool if reactor creation fails
        return 1;
    }

    // Add the server socket to the Reactor
    addFd(reactor, serverSocket, handleNewConnection);

    // Start the Reactor
    startReactor(reactor);

    // Wait for the Reactor to finish
    WaitFor(reactor);

    // Clean up
    threadpool_destroy(pool);
    stopReactor(reactor);
    close(serverSocket);

    return 0;
}