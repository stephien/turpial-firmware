#include "WsServer.h"
#include <errno.h>
#include <string.h>


namespace server {
QueueHandle_t client_queue;
QueueHandle_t persisten_queue;
const int DELAY = 1000 / portTICK_PERIOD_MS; // 1 second

WsServer::WsServer()
{
    client_queue = xQueueCreate(client_queue_size, sizeof(struct netconn*));
    persisten_queue = xQueueCreate(client_queue_size, sizeof(struct netconn*));
    conn_ = new netconn();
    netconn_bind(conn_, NULL, port_);
}


WsServer::~WsServer()
{
    xQueueReset(client_queue);
}


//task server should have to run forever
void WsServer::runServer()
{
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
        exit(0);
    }

    // Now server is ready to listen and verification
    if ((listen(sockfd_, 5)) != 0) {
        exit(0);
    }
    len_ = sizeof(cli_addr_);

    //Waitting for new connections forever
    do {
        connfd_ = new int();
        *connfd_ = accept(sockfd_, (SA*)&cli_addr_, &len_);
        if (*connfd_ > 0) {
            ESP_LOGI("SERVER_LOG --63", "server acccept the client...\n");
            xQueueSendToBack(client_queue, &connfd_, portMAX_DELAY); //this queue contain all connected clients       
        } else {
            ESP_LOGI("SERVER_LOG--66", "server acccept failed...\n");
            delete connfd_;
            closesocket(*connfd_);
            exit(0);
        }
    } while (true);
    closesocket(sockfd_);
    delete connfd_;
}


/**
 * @brief this function read client_queue and handle connections to see headers
 * and know the type of request, it need to run forever loop 
 * 
 */


void WsServer::handleClients()
{
    const static char* TAG = "handleClients";
    int* conn;
    ESP_LOGI(TAG, "task starting---handle clients");
    for (;;) {
        xQueueReceive(client_queue, &conn, portMAX_DELAY);
        ESP_LOGI(TAG, "RECIBIENDO DATA DEL SOCKET------------------->>>>>>>");
        if (conn) {
            ESP_LOGI("tcpserver-->>90", "en la cola conn != de cero");
            xQueueSendToBack(persisten_queue, &conn, portMAX_DELAY); //this queue contain all future websocket clients
        }
        delete conn;
    }
    vTaskDelete(NULL);
}

/**
 * @brief this function create and start the server and task to read the queue to see new clients
 * 
 * 
 */

void WsServer::start()
{
    xTaskCreate(&callbackServerTask, "Server Task-->>", 8000, this, 9, NULL);  //running http server
    xTaskCreate(&callbackClientsTask, "Client Task-->>", 4000, this, 6, NULL); //listen new connections
}


void WsServer::setPort(int port)
{
    this->port_ = port;
}

void WsServer::setTimeout(int16_t time)
{
    this->timeout_ = time;
}


/**
 * @brief with the next command we can test the connection with server
 * 
 */

/*
curl -i --no-buffer -H "Connection: Upgrade" -H "Upgrade: websocket" -H "Host: 192.168.4.1:8080" -H "Origin: http://192.168.4.1:8080" -H "Sec-WebSocket-Key: SGVsbG8sIHdvcmxkIQ==" -H "Sec-WebSocket-Version: 13" 192.168.4.1:8080/
*/

} // namespace server





/*


// serves any clients
void TcpServer::handleTcpConnections(int sockfd)
{
    ESP_LOGI("TEST", "HANDLE CONNECTION GOING TO TEST");
    const static char* TAG = "TCP Connections";
    static err_t err;

    char buff[MAX];
    int n;
    for (;;) {
        //  if (sockfd > 0) {
        bzero(buff, MAX);
        // read the message from client and copy it in buffer
        err = read(sockfd, buff, sizeof(buff));
        if (err != 0) {
            ESP_LOGI("TEST", "LOOP INFINITO HERE %d -- > %d", err, sockfd);

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
            //write(sockfd, buff, sizeof(buff));


            // if msg contains "Exit" then server exit and chat ended.
            if (strncmp("exit", buff, 4) == 0) {
                printf("Server Exit...\n");
                break;
            }
        }
        //}
    }
}


*/