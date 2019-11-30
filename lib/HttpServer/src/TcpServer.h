#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "Task.h"
#include "esp_log.h"
#include "lwip/api.h"
#include <cstdint>

namespace server {

const static char HTML_HEADER[] = "HTTP/1.1 200 OK\nContent-type: text/html\n\n";
static QueueHandle_t client_queue;


class TcpServer
{
public:
    TcpServer();
    virtual ~TcpServer();
    void setTimeout(int16_t time);
    void setPort(int port);
    //http server start
    void start(void);
    static const int socket_queue;
    
private:
    struct netconn* conn_;
    struct netconn* newconn_;
    struct netbuf* inbuf_;
    static char* buf_;
    static uint16_t buflen_;
    static err_t err_;
    int16_t timeout_;
    int port_ = 80;
    void handleTcpConnections(struct netconn* conn); //this function is called when any data is wrote in client_queue
    static const int client_queue_size = 5;
    void runServer(void);
    void handleClients(void);

    //wrapper to use member functions inside callbacks for tasks
    inline static void callbackServerTask(void* context) { static_cast<TcpServer*>(context)->runServer(); }
    //wrapper // to execute handleClients
    inline static void callbackClientsTask(void* context) { static_cast<TcpServer*>(context)->handleClients(); }

};

} // namespace server
#endif