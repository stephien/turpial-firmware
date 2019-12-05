#ifndef WSSERVER_H
#define WSSERVER_H

#include "Task.h"
#include "esp_log.h"
#include "lwip/api.h"
#include <cstdint>



#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "sdkconfig.h"

#define MAX 255
#define PORT 80
#define SA struct sockaddr

#define WEBSOCKET_SERVER_MAX_CLIENTS CONFIG_WEBSOCKET_SERVER_MAX_CLIENTS
#define WEBSOCKET_SERVER_QUEUE_TIMEOUT CONFIG_WEBSOCKET_SERVER_QUEUE_TIMEOUT


namespace server {

extern QueueHandle_t client_queue;
extern QueueHandle_t persisten_queue;


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
    void addClient();
    //wrapper to use member functions inside callbacks for tasks
    inline static void callbackServerTask(void* context) { static_cast<WsServer*>(context)->runServer(); }
    //wrapper // to execute handleClients
    inline static void callbackClientsTask(void* context) { static_cast<WsServer*>(context)->handleClients(); }
};

} // namespace server
#endif