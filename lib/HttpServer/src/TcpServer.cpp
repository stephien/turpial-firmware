#include "TcpServer.h"
#include "WebSocketServer.h"
#include <errno.h>
#include <string.h>

namespace server {
//extern QueueHandle_t client_queue;
QueueHandle_t persisten_queue;
const int DELAY = 1000 / portTICK_PERIOD_MS; // 1 second

TcpServer::TcpServer()
{
    client_queue = xQueueCreate(client_queue_size, sizeof(struct netconn*));
    persisten_queue = xQueueCreate(client_queue_size, sizeof(struct netconn*));
    conn_ = new netconn();
    netconn_bind(conn_, NULL, port_);
}


TcpServer::~TcpServer()
{
    xQueueReset(client_queue);
}


//task server should have to run forever
void TcpServer::runServer()
{
    const static char* TAG = "server_task";
    static err_t err;

    conn_ = netconn_new(NETCONN_TCP);
    netconn_bind(conn_, NULL, port_);
    netconn_listen(conn_);
    ESP_LOGI(TAG, "server listening");
    //Waitting for new connections forever
    do {
        //This function blocks the process until a connection request from a remote host arrives
        //on the TCP connection conn.
        err = netconn_accept(conn_, &newconn_);
        //when new connection, write this one inside client queue to be read in other different inner loop task
        //this queue will be read inside handleClients function
        if (err == ERR_OK) {
            ESP_LOGI(TAG, "new client");
            xQueueSendToBack(client_queue, &newconn_, portMAX_DELAY); //this queue contain all connected clients
            //http_serve(newconn);
        } else {
            netconn_close(conn_);
            netconn_delete(conn_);
            ESP_LOGE(TAG, "task ending,");
        }
    } while (true);
}


/**
 * @brief this function read client_queue and handle connections to see headers
 * and know the type of request, it need to run forever loop 
 * 
 */
void TcpServer::handleClients()
{
    const static char* TAG = "handleClients";
    struct netconn* conn;
    ESP_LOGI(TAG, "task starting---handle clients");
    for (;;) {
        xQueueReceive(client_queue, &conn, portMAX_DELAY);
        if (!conn) continue;
        handleTcpConnections(conn);
    }
    vTaskDelete(NULL);
}

/**
 * @brief this function create and start the server and task to read the queue to see new clients
 * 
 * 
 */

void TcpServer::start()
{
    xTaskCreate(&callbackServerTask, "Server Task-->>", 8000, this, 9, NULL);  //running http server
    xTaskCreate(&callbackClientsTask, "Client Task-->>", 4000, this, 6, NULL); //listen new connections
}


void TcpServer::setPort(int port)
{
    this->port_ = port;
}

void TcpServer::setTimeout(int16_t time)
{
    this->timeout_ = time;
}


// serves any clients
void TcpServer::handleTcpConnections(struct netconn* conn)
{
    const static char* TAG = "TCP Connections";
    static err_t err;

    typedef struct Temp {
        struct netconn* conn;
        struct netbuf* inbuf;
        char* buf;
        uint16_t buflen;
        err_t err;
    };
    Temp* connection = new Temp();
    connection->conn = conn;

    netconn_set_recvtimeout(conn, 1000); // allow a connection timeout of 1 second
    ESP_LOGI(TAG, "reading from client...");
    err = netconn_recv(conn, &connection->inbuf);
    ESP_LOGI(TAG, "read from client");

    if (err == ERR_OK) {
        netbuf_data(connection->inbuf, (void**)&connection->buf, &connection->buflen);
        if (connection->buf) {
            // default page for http
            if (strstr(connection->buf, "GET / ") && !strstr(connection->buf, "Upgrade: websocket")) {
                ESP_LOGI(TAG, "Sending /");
                netconn_write(connection->conn, HTML_HEADER, sizeof(HTML_HEADER) - 1, NETCONN_NOCOPY);
                netconn_write(connection->conn, "HI", 2, NETCONN_NOCOPY);
                netconn_close(connection->conn);
                netconn_delete(connection->conn);
                netbuf_delete(connection->inbuf);
            } else if (strstr(connection->buf, "GET / ") && strstr(connection->buf, "Upgrade: websocket")) { //check headers and upgrade to websockets
                ESP_LOGI(TAG, "Requesting websocket on /");
                //here we need to send a signal to read new socket connection inside websocket server class
                //wsServerAddClient(conn,buf,buflen,"/", websocket_callback);

                xQueueSendToBack(persisten_queue, &connection, portMAX_DELAY); //this queue contain all future websocket clients
                netbuf_delete(connection->inbuf);
            } else {
                netbuf_delete(connection->inbuf);
                ESP_LOGI(TAG, "Unknown request");
                netconn_close(connection->conn);
                netconn_delete(connection->conn);
                netbuf_delete(connection->inbuf);
            }
        } else { //if buf
            ESP_LOGI(TAG, "Unknown request (empty?...)");
            netconn_close(connection->conn);
            netconn_delete(connection->conn);
            netbuf_delete(connection->inbuf);
        }
    } else { // if err==ERR_OK
        ESP_LOGI(TAG, "error on read, closing connection");
        netconn_close(connection->conn);
        netconn_delete(connection->conn);
        netbuf_delete(connection->inbuf);
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