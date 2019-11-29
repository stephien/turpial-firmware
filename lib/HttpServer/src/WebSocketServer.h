#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include "Task.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "WebSocket.h"


namespace server {

#define WEBSOCKET_SERVER_MAX_CLIENTS CONFIG_WEBSOCKET_SERVER_MAX_CLIENTS
#define WEBSOCKET_SERVER_QUEUE_SIZE CONFIG_WEBSOCKET_SERVER_QUEUE_SIZE
#define WEBSOCKET_SERVER_QUEUE_TIMEOUT CONFIG_WEBSOCKET_SERVER_QUEUE_TIMEOUT
#define WEBSOCKET_SERVER_TASK_STACK_DEPTH CONFIG_WEBSOCKET_SERVER_TASK_STACK_DEPTH
#define WEBSOCKET_SERVER_TASK_PRIORITY CONFIG_WEBSOCKET_SERVER_TASK_PRIORITY
#define WEBSOCKET_SERVER_PINNED CONFIG_WEBSOCKET_SERVER_PINNED
#if WEBSOCKET_SERVER_PINNED
#define WEBSOCKET_SERVER_PINNED_CORE CONFIG_WEBSOCKET_SERVER_PINNED_CORE
#endif


class WebSocketServer : public Task
{
public:
    WebSocketServer();
    virtual ~WebSocketServer();
    // starts the server
    int ws_server_start();

    // ends the server
    int ws_server_stop();
    


    virtual void run(void* data) override
    {
        while (1)
            ;
    }

private:
};

} // namespace server
#endif