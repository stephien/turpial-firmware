#include "WsServer.h"
#include <errno.h>
#include <string.h>


#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"


#include "esp_system.h" // for esp_random
#include "mbedtls/base64.h"
#include "mbedtls/sha1.h"
#include <string.h>


namespace server {

QueueHandle_t client_queue;
QueueHandle_t persisten_queue;
static QueueHandle_t websocket_queue;                       // to hold the clients that send messages
static TaskHandle_t xtask;                                  // the task itself
static SemaphoreHandle_t websocket_mutex;                   // to lock the client array
static socket_client clients[WEBSOCKET_SERVER_MAX_CLIENTS]; // holds list of clients
const int DELAY = 1000 / portTICK_PERIOD_MS;                // 1 second

WsServer::WsServer()
{
    client_queue = xQueueCreate(client_queue_size, sizeof(int));
    persisten_queue = xQueueCreate(client_queue_size, sizeof(int));
}


WsServer::~WsServer()
{
    xQueueReset(client_queue);
}

// handles websocket events
void wsCallback(int conn, wsEvent event)
{
    ESP_LOGI("USING wsCALLBACK ", "THIS IS THE SOCKET WAS ADDED TO THE CLIENTS ARRAY------%d:)", conn);
    ESP_LOGI("USING wsCALLBACK ", "THIS IS THE SOCKET type %d", event);
    //we need to check for disconnected sockets
}

char* strnstr(const char* s, const char* find, size_t slen)
{
    char c, sc;
    size_t len;

    if ((c = *find++) != '\0') {
        len = strlen(find);
        do {
            do {
                if (slen-- < 1 || (sc = *s++) == '\0')
                    return (NULL);
            } while (sc != c);
            if (len > slen)
                return (NULL);
        } while (strncmp(s, find, len) != 0);
        s--;
    }
    return ((char*)s);
}

void WsServer::addClient(int conn, callBkAddClient callck)
{
    int i = 0;
    char buff[255];
    int n;
    ESP_LOGI("ENTRAMOS---->", "EN EL server %d", conn);
    read(conn, buff, sizeof(buff));
    printf("From client: %s\t To client : ", buff);

    if (strnstr(buff, WS_HEADER, MAX)) {
        unsigned char encoded_key[32];
        char key[64];
        char* key_start = strnstr(buff, WS_KEY, MAX);
        if (key_start) {
            key_start += 19;
            char* key_end = strnstr(key_start, "\r\n", MAX);
            if (key_end) {
                int len = sizeof(char) * (key_end - key_start);
                if (len + sizeof(WS_GUID) < sizeof(key) && len > 0) {
                    // Concatenate key
                    memcpy(key, key_start, len);
                    strlcpy(&key[len], WS_GUID, sizeof(key));
                    printf("Resulting key: %s\n", key);
                    // Get SHA1
                    unsigned char sha1sum[20];
                    mbedtls_sha1((unsigned char*)key, sizeof(WS_GUID) + len - 1, sha1sum);
                    //Base64 encode
                    unsigned int olen;
                    mbedtls_base64_encode(NULL, 0, &olen, sha1sum, 20); //get length
                    int ok = mbedtls_base64_encode(encoded_key, sizeof(encoded_key), &olen, sha1sum, 20);
                    if (ok == 0) {
                        // hs->is_websocket = 1;
                        encoded_key[olen] = '\0';
                        printf("Base64 encoded: %s\n", encoded_key);
                        // Send response
                        char buf[256];
                        uint16_t len = snprintf(buf, sizeof(buf), WS_RSP, encoded_key);
                        write(conn, buf, sizeof(buf)); //send the reply to switch to ws

                        for (int i = 0; i < WEBSOCKET_SERVER_MAX_CLIENTS; i++) {
                            if (!clients[i].conn) { //if the index in array is free we can alloate the socket
                                clients[i].conn = conn;
                                clients[i].isConnected = true;
                                break;
                            }
                        }
                    }
                } else {
                    printf("Key overflow\n");
                    //return ERR_MEM;
                }
            }
        } else {
            printf("Malformed packet\n");
            //return ERR_ARG;
        }
    }
}


//task server should have to run forever
void WsServer::runServer()
{
    // socket create and verification
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    websocket_mutex = xSemaphoreCreateMutex();

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
    connfd_ = new int();

    ESP_LOGI("after loop", "task starting---handle clients");
    for (int i = 0; i < WEBSOCKET_SERVER_MAX_CLIENTS; i++) {
        clients[i].conn = 0;
    }
    //Waitting for new connections forever
    do {
        *connfd_ = accept(sockfd_, (SA*)&cli_addr_, &len_);
        if (*connfd_ > 0) {
            ESP_LOGI("SERVER_LOG --63", "server acccept the client...\n");
            xSemaphoreTake(websocket_mutex, portMAX_DELAY);
            addClient(*connfd_, wsCallback); //try to switch client and add to array
            xQueueSendToBack(client_queue, &connfd_, portMAX_DELAY); //signal to emit new client connected
            xSemaphoreGive(websocket_mutex);
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

void WsServer::readMessage(int i)
{
    ESP_LOGI("LEYENDO----->", "socket %d", i);
    struct netconn* conn;
}


void WsServer::handleClients()
{
    const static char* TAG = "handleClients";
    int* conn;
    char buff[200];

    int n;
    static err_t err;

    ESP_LOGI(TAG, "task starting---handle clients");
    for (;;) {
        xQueueReceive(client_queue, &conn, portMAX_DELAY);
        if (*conn != 0) {
            xSemaphoreTake(websocket_mutex, portMAX_DELAY);
            //here we have array of clients to send a receive messages and the actual client in queue
            ESP_LOGI("ENTRAMOS---->", "RECIBIMOS EN LA QUEUE %d", *conn);

            xSemaphoreGive(websocket_mutex);
        }
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


/**
 * @brief with the next command we can test the connection with server
 * 
 */

/*
curl -i --no-buffer -H "Connection: Upgrade" -H "Upgrade: websocket" -H "Host: 192.168.4.1:8080" -H "Origin: http://192.168.4.1:8080" -H "Sec-WebSocket-Key: SGVsbG8sIHdvcmxkIQ==" -H "Sec-WebSocket-Version: 13" 192.168.4.1:8080/
*/

} // namespace server
