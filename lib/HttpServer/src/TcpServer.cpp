#include "TcpServer.h"
#include "WebSocketServer.h"
#include <errno.h>
#include <string.h>


namespace server {
QueueHandle_t client_queue;
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


/* //task server should have to run forever
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
 */

//task server should have to run forever
void TcpServer::runServer()
{
    const static char* TAG = "server_task";
    static err_t err;
    // socket create and verification
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ < 0) {
        ESP_LOGI("*****************", "was not possible to create the socket\n");
        exit(0);
    } else {
        ESP_LOGI("*****************", "Socket successfully created..\n");
        bzero(&serveraddr_, sizeof(serveraddr_));
    }
    // assign IP, PORT
    serveraddr_.sin_family = AF_INET;
    serveraddr_.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr_.sin_port = htons(PORT);


    // Binding newly created socket to given IP and verification
    if ((bind(sockfd_, (SA*)&serveraddr_, sizeof(serveraddr_))) != 0) {
        ESP_LOGI("******************", "socket bind failed...\n");
        exit(0);
    } else
        ESP_LOGI("**************************", "Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd_, 5)) != 0) {
        ESP_LOGI("*****************************", "Listen failed...\n");
        exit(0);
    } else
        ESP_LOGI("*****************************", "Server listening..\n");
    len_ = sizeof(cli_addr_);

    //Waitting for new connections forever
    do {
        connfd_ = new int();
        *connfd_ = accept(sockfd_, (SA*)&cli_addr_, &len_);
        // ESP_LOGI("VALUE OF SOCKET FILE DESCRIPTOR CONECTION:   ", "Value: %d", *connfd_);
        if (*connfd_ > 0) {
            printf("server acccept the client...\n");
            xQueueSendToBack(client_queue, &connfd_, portMAX_DELAY); //this queue contain all connected clients
        } else {
            printf("server acccept failed...\n");
            delete connfd_;
            closesocket(*connfd_);
            exit(0);
        }
    } while (true);
    closesocket(sockfd_);
}


/**
 * @brief this function read client_queue and handle connections to see headers
 * and know the type of request, it need to run forever loop 
 * 
 */
/* void TcpServer::handleClients()
{
    const static char* TAG = "handleClients";
    struct netconn* conn;
    ESP_LOGI(TAG, "task starting---handle clients");
    for (;;) {
        xQueueReceive(client_queue, &conn, portMAX_DELAY);
         if (conn) continue;
         handleTcpConnections(conn); 
    }
    vTaskDelete(NULL);
} */

void TcpServer::handleClients()
{
    const static char* TAG = "handleClients";
    int* conn;
    ESP_LOGI(TAG, "task starting---handle clients");
    for (;;) {
        xQueueReceive(client_queue, &conn, portMAX_DELAY);
        ESP_LOGI(TAG, "RECIBIENDO DATA DEL SOCKET------------------->>>>>>>");
        if (conn) {
            handleTcpConnections(*conn);
            xQueueSendToBack(persisten_queue, &conn, portMAX_DELAY); //this queue contain all future websocket clients
            closesocket(*conn);
            delete conn;
        }
        delete conn;
        //handleTcpConnections(*conn);
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


/* // serves any clients
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
} */

// serves any clients
void TcpServer::handleTcpConnections(int sockfd)
{
    ESP_LOGI("TEST", "HANDLE CONNECTION GOING TO TEST");
    const static char* TAG = "TCP Connections";
    static err_t err;

    char buff[MAX];
    int n;
    for (;;) {
        if (sockfd) {
            bzero(buff, MAX);
            // read the message from client and copy it in buffer
            err = read(sockfd, buff, sizeof(buff));
            if (err > 0) {
                // print buffer which contains the client contents
                printf("From client: %s\t To client : ", buff);
                bzero(buff, MAX);
                n = 0;
                while (n < MAX) {
                    buff[n] = getchar();
                    if (buff[n] == '\n') break;
                    n += 1;
                }
                // and send that buffer to client
                write(sockfd, buff, sizeof(buff));


                // if msg contains "Exit" then server exit and chat ended.
                if (strncmp("exit", buff, 4) == 0) {
                    printf("Server Exit...\n");
                    break;
                }
            } else {
                closesocket(sockfd);
            }
        }
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