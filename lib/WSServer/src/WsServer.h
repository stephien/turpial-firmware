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
#define MAX 255
#define PORT 80
#define SA struct sockaddr



namespace server {

typedef enum {
    WEBSOCKET_CONNECT,
    WEBSOCKET_DISCONNECT_EXTERNAL, // the other side disconnected
    WEBSOCKET_DISCONNECT_INTERNAL, // the esp32 disconnected
    WEBSOCKET_DISCONNECT_ERROR,    // disconnect due to error
    WEBSOCKET_TEXT,
    WEBSOCKET_BIN,
    WEBSOCKET_PING,
    WEBSOCKET_PONG
} WEBSOCKET_TYPE_t;

// websocket operation codes
typedef enum {
    WEBSOCKET_OPCODE_CONT = 0x0,
    WEBSOCKET_OPCODE_TEXT = 0x1,
    WEBSOCKET_OPCODE_BIN = 0x2,
    WEBSOCKET_OPCODE_CLOSE = 0x8,
    WEBSOCKET_OPCODE_PING = 0x9,
    WEBSOCKET_OPCODE_PONG = 0xA
} WEBSOCKET_OPCODES_t;

typedef struct {
    struct netconn* conn;                                                           // the connection
    char* url;                                                                      // the associated url,  null terminated
    char* protocol;                                                                 // the associated protocol, null terminated
    bool ping;                                                                      // did we send a ping?
    WEBSOCKET_OPCODES_t last_opcode;                                                // the previous opcode, except a continuation frame
    char* contin;                                                                   // any continuation piece
    bool contin_text;                                                               // is the continue a binary or text?
    uint64_t len;                                                                   // length of continuation
    void (*ccallback)(WEBSOCKET_TYPE_t type, char* msg, uint64_t len);              // client callback
    void (*scallback)(uint8_t num, WEBSOCKET_TYPE_t type, char* msg, uint64_t len); // server callback
} ws_client_t;


const static char HTML_HEADER[] = "HTTP/1.1 200 OK\nContent-type: text/html\n\n";
/* static QueueHandle_t client_queue;
static QueueHandle_t persisten_queue; */
extern QueueHandle_t client_queue;
extern QueueHandle_t persisten_queue;


class WsServer
{
public:
    WsServer();
    virtual ~WsServer();
    void setTimeout(int16_t time);
    void setPort(int port);
    //http server start
    void start(void);
    static const int socket_queue;
    
private:

    int sockfd_, *connfd_;
    socklen_t len_;
    struct sockaddr_in serveraddr_, cli_addr_;



    struct netconn* conn_;
    struct netconn* newconn_;
    struct netbuf* inbuf_;
    static char* buf_;
    static uint16_t buflen_;
    static err_t err_;
    int16_t timeout_;
    int port_ = 80;
    //void handleTcpConnections(struct netconn* conn); //this function is called when any data is wrote in client_queue
    
    
     void handleTcpConnections(int conn);
    static const int client_queue_size = 5;
    void runServer(void);
    void handleClients(void);

    //wrapper to use member functions inside callbacks for tasks
    inline static void callbackServerTask(void* context) { static_cast<WsServer*>(context)->runServer(); }
    //wrapper // to execute handleClients
    inline static void callbackClientsTask(void* context) { static_cast<WsServer*>(context)->handleClients(); }
    void deleteConnectionObject(netconn* conn);
};

} // namespace server
#endif