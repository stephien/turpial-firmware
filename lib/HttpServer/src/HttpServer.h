#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "Task.h"
#include "esp_log.h"
#include "lwip/api.h"
#include <cstdint>

namespace server {

const static char HTML_HEADER[] = "HTTP/1.1 200 OK\nContent-type: text/html\n\n";
static QueueHandle_t client_queue;


class HttpServer
{
public:
    HttpServer();
    virtual ~HttpServer();
    void setTimeout(int16_t time);
    void setPort(int port);
    void handleClients(void);
    void start(void);
    void runServer(void);

private:
    struct netconn* conn_;
    struct netconn* newconn_;
    struct netbuf* inbuf_;
    static char* buf_;
    static uint16_t buflen_;
    static err_t err_;
    int16_t timeout_;
    int port_ = 80;
    void handleHttpConnections(struct netconn* conn);
    static const int client_queue_size = 5;
};

} // namespace server
#endif