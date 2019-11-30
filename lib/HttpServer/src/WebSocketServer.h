#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H


#include "WebSocket.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "TcpServer.h"


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


class WebSocketServer: WebSocket
{
public:
    WebSocketServer();
    virtual ~WebSocketServer();
    // starts the server
    int ws_server_start();

    // ends the server
    int ws_server_stop();
    //void handleRead(uint8_t num);

    // adds a client, returns the client's number in the server
    int ws_server_add_client(struct netconn* conn,
        char* msg,
        uint16_t len,
        char* url,
        void (*callback)(uint8_t num,
            WEBSOCKET_TYPE_t type,
            char* msg,
            uint64_t len));

    int ws_server_len_url(char* url); // returns the number of connected clients to url
    int ws_server_len_all();          // returns the total number of connected clients

    int ws_server_remove_client(int num);    // removes the client with the set number
    int ws_server_remove_clients(char* url); // removes all clients connected to the specified url
    int ws_server_remove_all();              // removes all clients from the server

    int ws_server_send_text_client(int num, char* msg, uint64_t len);    // send text to client with the set number
    int ws_server_send_text_clients(char* url, char* msg, uint64_t len); // sends text to all clients with the set number
    int ws_server_send_text_all(char* msg, uint64_t len);                // sends text to all clients

    int ws_server_send_bin_client(int num, char* msg, uint64_t len);
    int ws_server_send_bin_clients(char* url, char* msg, uint64_t len);
    int ws_server_send_bin_all(char* msg, uint64_t len);

    // these versions can be sent from the callback ONLY

    int ws_server_send_text_client_from_callback(int num, char* msg, uint64_t len);    // send text to client with the set number
    int ws_server_send_text_clients_from_callback(char* url, char* msg, uint64_t len); // sends text to all clients with the set number
    int ws_server_send_text_all_from_callback(char* msg, uint64_t len);                // sends text to all clients

    int ws_server_ping(); // sends a ping to all connected clients



    void handle_read(uint8_t num);
    bool prepare_response(char* buf,uint32_t buflen,char* handshake); 

    static void ws_server_task(void* pvParameters);

    void handleWebsocket();


private:
};

} // namespace server
#endif