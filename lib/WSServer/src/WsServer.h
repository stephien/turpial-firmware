#ifndef WSSERVER_H
#define WSSERVER_H

#include "Task.h"
#include "esp_log.h"
#include "lwip/api.h"
#include <cstdint>


#include "sdkconfig.h"
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MAX 255
#define PORT 8080
#define SA struct sockaddr

#define WEBSOCKET_SERVER_MAX_CLIENTS CONFIG_WEBSOCKET_SERVER_MAX_CLIENTS
#define WEBSOCKET_SERVER_QUEUE_TIMEOUT CONFIG_WEBSOCKET_SERVER_QUEUE_TIMEOUT

namespace server {


const char WS_HEADER[] = "Upgrade: websocket\r\n";
const char WS_GUID[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
const char WS_KEY[] = "Sec-WebSocket-Key: ";
const char WS_RSP[] = "HTTP/1.1 101 Switching Protocols\r\n"
                      "Upgrade: websocket\r\n"
                      "Connection: Upgrade\r\n"
                      "Sec-WebSocket-Accept: %s\r\n\r\n";

extern QueueHandle_t client_queue;
extern QueueHandle_t persisten_queue;

typedef struct {
    int conn;
    bool isConnected = false;
    void (*clienteCallback)(int conn);
    void (*serverCallback)(uint8_t index, char* msg, uint64_t len);
} socket_client;

typedef enum {
    CONNECTED,
    DISCONNECT,
    TEXT,
    BIN,
    PING,
    PONG
} wsEvent;


typedef void (*callBkAddClient)(int conn, wsEvent event);

class WsServer
{
public:
    WsServer();
    virtual ~WsServer();
    //http server start
    void start(void);
    static const int socket_queue;

private:
    int sockfd_, *connfd_;
    socklen_t len_;
    struct sockaddr_in serveraddr_, cli_addr_;
    static const int client_queue_size = 5;

    void runServer(void);
    void handleClients(void);
    //void addClient(int conn);
    void addClient(int conn, callBkAddClient);

    void readMessage(int i);
    //wrapper to use member functions inside callbacks for tasks
    inline static void callbackServerTask(void* context) { static_cast<WsServer*>(context)->runServer(); }
    //wrapper // to execute handleClients
    inline static void callbackClientsTask(void* context) { static_cast<WsServer*>(context)->handleClients(); }
};

} // namespace server
#endif