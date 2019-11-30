#include "HttpServer.h"
#include "WebSocketServer.h"
#include <errno.h>
#include <string.h>

namespace server {
const int DELAY = 1000 / portTICK_PERIOD_MS; // 1 second

HttpServer::HttpServer()
{
    client_queue = xQueueCreate(client_queue_size, sizeof(struct netconn*));
    conn_ = new netconn();
    netconn_bind(conn_, NULL, port_);
}


HttpServer::~HttpServer()
{
  xQueueReset(client_queue);
}

//wrapper to use member functions inside callbacks for tasks
static void runServerTask(void* instance)
{
    static_cast<HttpServer*>(instance)->runServer(); //calling member function class inside callback Task freeRTOS
}
//task server should have to run forever
void HttpServer::runServer()
{
    const static char* TAG = "server_task";
    static err_t err;
    //client_queue = xQueueCreate(client_queue_size, sizeof(struct netconn*));

    conn_ = netconn_new(NETCONN_TCP);
    netconn_bind(conn_, NULL, port_);
    netconn_listen(conn_);
    ESP_LOGI(TAG, "server listening");
    //Waitting for new connections forever
    do {
        err = netconn_accept(conn_, &newconn_);
        //when new connection write this one inside client queue to be read in other inner loop task
        //this queue will be read inside handleClients function
        if (err == ERR_OK) {
            ESP_LOGI(TAG, "new client");
            xQueueSendToBack(client_queue, &newconn_, portMAX_DELAY);
            //http_serve(newconn);
        } else {
            netconn_close(conn_);
            netconn_delete(conn_);
            ESP_LOGE(TAG, "task ending,");
        }
    } while (true);
}


//wrapper // to execute handleClients 
void handleClientsTask(void* instance)
{
    static_cast<HttpServer*>(instance)->handleClients();
}

/**
 * @brief this function read client_queue and handle connections to see headers
 * and know the type of request
 * 
 */
void HttpServer::handleClients()
{
    const static char* TAG = "handleClients";
    struct netconn* conn;
    ESP_LOGI(TAG, "task starting");
    for (;;) {
        xQueueReceive(client_queue, &conn, portMAX_DELAY);
        if (!conn) continue;
        handleHttpConnections(conn);
    }
    vTaskDelete(NULL);
}

void HttpServer::start()
{
    xTaskCreate(&runServerTask, "TaskServer", 4000, this, 9, NULL);      //running http server
    xTaskCreate(&handleClientsTask, "Client-Task", 4000, this, 6, NULL); //listen new connections
}




void HttpServer::setPort(int port)
{
    this->port_ = port;
}

void HttpServer::setTimeout(int16_t time)
{
    this->timeout_ = time;
}


// serves any clients
void HttpServer::handleHttpConnections(struct netconn* conn)
{
    const static char* TAG = "http_server";
    const static char HTML_HEADER[] = "HTTP/1.1 200 OK\nContent-type: text/html\n\n";

    struct netbuf* inbuf;
    static char* buf;
    static uint16_t buflen;
    static err_t err;


    netconn_set_recvtimeout(conn, 1000); // allow a connection timeout of 1 second
    ESP_LOGI(TAG, "reading from client...");
    err = netconn_recv(conn, &inbuf);
    ESP_LOGI(TAG, "read from client");
    if (err == ERR_OK) {
        netbuf_data(inbuf, (void**)&buf, &buflen);
        if (buf) {
            if (strstr(buf, "GET / ") && strstr(buf, "Upgrade: websocket")) {
                ESP_LOGI(TAG, "Requesting websocket on /");
                //wsServerAddClient(conn,buf,buflen,"/", websocket_callback);
                //netbuf_delete(inbuf);
            } else { //esto se hizo para que funcione el wensocket javascript solamente
                netbuf_delete(inbuf);
                ESP_LOGI(TAG,"Unknown request");
                netconn_close(conn);
                netconn_delete(conn);
                netbuf_delete(inbuf);
            }
        } else { //if buf
            ESP_LOGI(TAG, "Unknown request (empty?...)");
            netconn_close(conn);
            netconn_delete(conn);
            netbuf_delete(inbuf);
        }
    } else { // if err==ERR_OK
        ESP_LOGI(TAG, "error on read, closing connection");
        netconn_close(conn);
        netconn_delete(conn);
        netbuf_delete(inbuf);
    }
}

/**
 * @brief with the next command we can test the connection with server
 * 
 */

/*
curl -i --no-buffer -H "Connection: Upgrade" -H "Upgrade: websocket" -H "Host: 192.168.4.1:8080" -H "Origin: http://192.168.4.1:8080" -H "Sec-WebSocket-Key: SGVsbG8sIHdvcmxkIQ==" -H "Sec-WebSocket-Version: 13" 192.168.4.1:8080/
*/

} // namespace server