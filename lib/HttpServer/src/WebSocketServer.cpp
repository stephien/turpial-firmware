#include "WebSocketServer.h"
#include <errno.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"


namespace server {

WebSocketServer::WebSocketServer()
{
    persisten_queue = xQueueCreate(10, sizeof(struct netconn*));
}

WebSocketServer::~WebSocketServer()
{
}


static SemaphoreHandle_t xwebsocket_mutex;                // to lock the client array
static QueueHandle_t xwebsocket_queue;                    // to hold the clients that send messages
static ws_client_t clients[WEBSOCKET_SERVER_MAX_CLIENTS]; // holds list of clients
static TaskHandle_t xtask;                                // the task itself

static void background_callback(struct netconn* conn, enum netconn_evt evt, u16_t len)
{
    switch (evt) {
    case NETCONN_EVT_RCVPLUS:
        xQueueSendToBack(xwebsocket_queue, &conn, WEBSOCKET_SERVER_QUEUE_TIMEOUT);
        break;
    default:
        break;
    }
}

// handles websocket events
void websocket_callback(uint8_t num, WEBSOCKET_TYPE_t type, char* msg, uint64_t len)
{
    const static char* TAG = "websocket_callback";
    int value;

    switch (type) {
    case WEBSOCKET_CONNECT:
        ESP_LOGI(TAG, "client %i connected!", num);
        break;
    case WEBSOCKET_DISCONNECT_EXTERNAL:
        ESP_LOGI(TAG, "client %i sent a disconnect message", num);
        break;
    case WEBSOCKET_DISCONNECT_INTERNAL:
        ESP_LOGI(TAG, "client %i was disconnected", num);
        break;
    case WEBSOCKET_DISCONNECT_ERROR:
        ESP_LOGI(TAG, "client %i was disconnected due to an error", num);
        break;
    case WEBSOCKET_TEXT:
        if (len) {
            switch (msg[0]) {
            case 'L':
                if (sscanf(msg, "L%i", &value)) {
                    ESP_LOGI(TAG, "LED value: %i", value);
                    //ws_server_send_text_all_from_callback(msg,len); // broadcast it!
                }
            }
        }
        break;
    case WEBSOCKET_BIN:
        ESP_LOGI(TAG, "client %i sent binary message of size %i:\n%s", num, (uint32_t)len, msg);
        break;
    case WEBSOCKET_PING:
        ESP_LOGI(TAG, "client %i pinged us with message of size %i:\n%s", num, (uint32_t)len, msg);
        break;
    case WEBSOCKET_PONG:
        ESP_LOGI(TAG, "client %i responded to the ping", num);
        break;
    }
}

//read persistent queue
static void ws_handleWebsocket_task(void* instance)
{
    
    const static char* TAG = "handleClients";
    /* struct
    {
        struct netconn* conn;
        struct netbuf* inbuf;
        char* buf;
        uint16_t buflen;
        err_t err;
    } connectionT; */
    struct netconn* conn;

    //struct netconn* conn;
    ESP_LOGI(TAG, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!task starting");
    for (;;) {
        xQueueReceive(server::persisten_queue, &conn, portMAX_DELAY); //receive clients from Tcp Server
         ESP_LOGI("WEBSOCKET NEW", "YES NEW WEBSOCKET EVENT CONNECTED!!!!!!!!!!"); 
      /*   if (!connectionT.conn) continue;
        ESP_LOGI("WEBSOCKET NEW", "YES NEW WEBSOCKET EVENT CONNECTED!!!!!!!!!!"); */
        //static_cast<WebSocketServer*>(instance)->ws_server_add_client(connection.conn, connection.buf, connection.buflen, "/", websocket_callback);
    }
    vTaskDelete(NULL);
}

void write_queue(void* context) {
    struct netconn* temp;
    for(;;) {
     xQueueSendToBack(persisten_queue,&temp , portMAX_DELAY);
     vTaskDelay(1000 / portTICK_RATE_MS);
    
    }
}

void WebSocketServer::handle_read(uint8_t num)
{
    ws_header_t header;
    char* msg;

    header.received = 0;
    msg = wsRead(&clients[num], &header);
    if (!header.received) return;

    switch (clients[num].last_opcode) {
    case WEBSOCKET_OPCODE_CONT:
        break;
    case WEBSOCKET_OPCODE_BIN:
        clients[num].scallback(num, WEBSOCKET_BIN, msg, header.length);
        break;
    case WEBSOCKET_OPCODE_TEXT:
        clients[num].scallback(num, WEBSOCKET_TEXT, msg, header.length);
        break;
    case WEBSOCKET_OPCODE_PING:
        wsSend(&clients[num], WEBSOCKET_OPCODE_PONG, msg, header.length, 0);
        clients[num].scallback(num, WEBSOCKET_PING, msg, header.length);
        break;
    case WEBSOCKET_OPCODE_PONG:
        if (clients[num].ping) {
            clients[num].scallback(num, WEBSOCKET_PONG, NULL, 0);
            clients[num].ping = 0;
        }
        break;
    case WEBSOCKET_OPCODE_CLOSE:
        clients[num].scallback(num, WEBSOCKET_DISCONNECT_EXTERNAL, NULL, 0);
        wsDisconnectClient(&clients[num]);
        break;
    default:
        break;
    }
    if (msg) free(msg);
}

static void ws_server_task(void* instance)
{
    struct netconn* conn;

    ESP_LOGI("-------------------------------------------------WS_SERVER_TASK", "FUNCIONANDO OK----------------------------------------");
    xwebsocket_mutex = xSemaphoreCreateMutex();
    xwebsocket_queue = xQueueCreate(WEBSOCKET_SERVER_QUEUE_SIZE, sizeof(struct netconn*));

    // initialize all clients
    for (int i = 0; i < WEBSOCKET_SERVER_MAX_CLIENTS; i++) {
        clients[i].conn = NULL;
        clients[i].url = NULL;
        clients[i].ping = 0;
        clients[i].last_opcode = static_cast<WEBSOCKET_OPCODES_t>(0);
        clients[i].contin = NULL;
        clients[i].len = 0;
        clients[i].ccallback = NULL;
        clients[i].scallback = NULL;
    }

    for (;;) {
        xQueueReceive(xwebsocket_queue, &conn, portMAX_DELAY);
        if (!conn) continue;                             // if the connection was NULL, ignore it
        xSemaphoreTake(xwebsocket_mutex, portMAX_DELAY); // take access
        for (int i = 0; i < WEBSOCKET_SERVER_MAX_CLIENTS; i++) {
            if (clients[i].conn == conn) {
                static_cast<WebSocketServer*>(instance)->handle_read(i); //calling WebsocketServer members from static function
                break;
            }
        }
        xSemaphoreGive(xwebsocket_mutex); // return access
    }
    vTaskDelete(NULL);
}

int WebSocketServer::ws_server_start()
{
    if (xtask) return 0;
    xTaskCreate(&ws_server_task,
        "ws_server_task",
        WEBSOCKET_SERVER_TASK_STACK_DEPTH,
        this,
        WEBSOCKET_SERVER_TASK_PRIORITY,
        &xtask);
  
     xTaskCreate(&ws_handleWebsocket_task,"handle_websocketTask",3000,this,5,NULL);
    // xTaskCreate(&write_queue,"handle_websocketTask",3000,this,5,NULL);
      return 1; 
}

int WebSocketServer::ws_server_stop()
{
    if (!xtask) return 0;
    vTaskDelete(xtask);
    return 1;
}

bool WebSocketServer::prepare_response(char* buf, uint32_t buflen, char* handshake)
{
    const char WS_HEADER[] = "Upgrade: websocket\r\n";
    const char WS_KEY[] = "Sec-WebSocket-Key: ";
    const char WS_RSP[] = "HTTP/1.1 101 Switching Protocols\r\n"
                          "Upgrade: websocket\r\n"
                          "Connection: Upgrade\r\n"
                          "Sec-WebSocket-Accept: %s\r\n\r\n";

    char* key_start;
    char* key_end;
    char* hashed_key;

    if (!strstr(buf, WS_HEADER)) return 0;
    if (!buflen) return 0;
    key_start = strstr(buf, WS_KEY);
    if (!key_start) return 0;
    key_start += 19;
    key_end = strstr(key_start, "\r\n");
    if (!key_end) return 0;

    hashed_key = wsHashHandshake(key_start, key_end - key_start);
    if (!hashed_key) return 0;
    sprintf(handshake, WS_RSP, hashed_key);
    return 1;
}

int WebSocketServer::ws_server_add_client(struct netconn* conn,
    char* msg,
    uint16_t len,
    char* url,
    void (*callback)(uint8_t num,
        WEBSOCKET_TYPE_t type,
        char* msg,
        uint64_t len))
{
    int ret;
    char handshake[256];

    if (!prepare_response(msg, len, handshake)) {
        netconn_close(conn);
        netconn_delete(conn);
        return -2;
    }


    ret = -1;
    xSemaphoreTake(xwebsocket_mutex, portMAX_DELAY);


    conn->callback = background_callback;
    netconn_write(conn, handshake, strlen(handshake), NETCONN_COPY);
    for (int i = 0; i < WEBSOCKET_SERVER_MAX_CLIENTS; i++) {
        if (clients[i].conn) continue;
        callback(i, WEBSOCKET_CONNECT, NULL, 0);
        clients[i] = wsConnectClient(conn, url, NULL, callback);
        if (!wsIsConnected(clients[i])) {
            callback(i, WEBSOCKET_DISCONNECT_ERROR, NULL, 0);
            wsDisconnectClient(&clients[i]);
            break;
        }
        ret = i;
        break;
    }

    xSemaphoreGive(xwebsocket_mutex);
    return ret;
}

int WebSocketServer::ws_server_len_url(char* url)
{
    int ret;
    ret = 0;
    xSemaphoreTake(xwebsocket_mutex, portMAX_DELAY);
    for (int i = 0; i < WEBSOCKET_SERVER_MAX_CLIENTS; i++) {
        if (clients[i].url && strcmp(url, clients[i].url)) ret++;
    }
    xSemaphoreGive(xwebsocket_mutex);
    return ret;
}

int WebSocketServer::ws_server_len_all()
{
    int ret;
    ret = 0;
    xSemaphoreTake(xwebsocket_mutex, portMAX_DELAY);
    for (int i = 0; i < WEBSOCKET_SERVER_MAX_CLIENTS; i++) {
        if (clients[i].conn) ret++;
    }
    xSemaphoreGive(xwebsocket_mutex);
    return ret;
}

int WebSocketServer::ws_server_remove_client(int num)
{
    int ret = 0;
    xSemaphoreTake(xwebsocket_mutex, portMAX_DELAY);
    if (wsIsConnected(clients[num])) {
        clients[num].scallback(num, WEBSOCKET_DISCONNECT_INTERNAL, NULL, 0);
        wsDisconnectClient(&clients[num]);
        ret = 1;
    }
    xSemaphoreGive(xwebsocket_mutex);
    return ret;
}

int WebSocketServer::ws_server_remove_clients(char* url)
{
    int ret = 0;
    xSemaphoreTake(xwebsocket_mutex, portMAX_DELAY);
    for (int i = 0; i < WEBSOCKET_SERVER_MAX_CLIENTS; i++) {
        if (wsIsConnected(clients[i]) && strcmp(url, clients[i].url)) {
            clients[i].scallback(i, WEBSOCKET_DISCONNECT_INTERNAL, NULL, 0);
            wsDisconnectClient(&clients[i]);
            ret += 1;
        }
    }
    xSemaphoreGive(xwebsocket_mutex);
    return ret;
}

int WebSocketServer::ws_server_remove_all()
{
    int ret = 0;
    xSemaphoreTake(xwebsocket_mutex, portMAX_DELAY);
    for (int i = 0; i < WEBSOCKET_SERVER_MAX_CLIENTS; i++) {
        if (wsIsConnected(clients[i])) {
            clients[i].scallback(i, WEBSOCKET_DISCONNECT_INTERNAL, NULL, 0);
            wsDisconnectClient(&clients[i]);
            ret += 1;
        }
    }
    xSemaphoreGive(xwebsocket_mutex);
    return ret;
}

// The following functions are already written below, but without the mutex.

int WebSocketServer::ws_server_send_text_client(int num, char* msg, uint64_t len)
{
    xSemaphoreTake(xwebsocket_mutex, portMAX_DELAY);
    int ret = ws_server_send_text_client_from_callback(num, msg, len);
    xSemaphoreGive(xwebsocket_mutex);
    return ret;
}

int WebSocketServer::ws_server_send_text_clients(char* url, char* msg, uint64_t len)
{
    xSemaphoreTake(xwebsocket_mutex, portMAX_DELAY);
    int ret = ws_server_send_text_clients_from_callback(url, msg, len);
    xSemaphoreGive(xwebsocket_mutex);
    return ret;
}

int WebSocketServer::ws_server_send_text_all(char* msg, uint64_t len)
{
    xSemaphoreTake(xwebsocket_mutex, portMAX_DELAY);
    int ret = ws_server_send_text_all_from_callback(msg, len);
    xSemaphoreGive(xwebsocket_mutex);
    return ret;
}

// the following functions should be used inside of the callback. The regular versions
// grab the mutex, but it is already grabbed from inside the callback so it will hang.

int WebSocketServer::ws_server_send_text_client_from_callback(int num, char* msg, uint64_t len)
{
    int ret = 0;
    if (wsIsConnected(clients[num])) {
        wsSend(&clients[num], WEBSOCKET_OPCODE_TEXT, msg, len, 0);
        ret = 1;
        if (!wsIsConnected(clients[num])) {
            clients[num].scallback(num, WEBSOCKET_DISCONNECT_ERROR, NULL, 0);
            wsDisconnectClient(&clients[num]);
            ret = 0;
        }
    }
    return ret;
}

int WebSocketServer::ws_server_send_text_clients_from_callback(char* url, char* msg, uint64_t len)
{
    int ret = 0;
    for (int i = 0; i < WEBSOCKET_SERVER_MAX_CLIENTS; i++) {
        if (wsIsConnected(clients[i]) && strcmp(clients[i].url, url)) {
            wsSend(&clients[i], WEBSOCKET_OPCODE_TEXT, msg, len, 0);
            if (wsIsConnected(clients[i]))
                ret += 1;
            else {
                clients[i].scallback(i, WEBSOCKET_DISCONNECT_ERROR, NULL, 0);
                wsDisconnectClient(&clients[i]);
            }
        }
    }
    return ret;
}

int WebSocketServer::ws_server_send_text_all_from_callback(char* msg, uint64_t len)
{
    int ret = 0;
    for (int i = 0; i < WEBSOCKET_SERVER_MAX_CLIENTS; i++) {
        if (wsIsConnected(clients[i])) {
            wsSend(&clients[i], WEBSOCKET_OPCODE_TEXT, msg, len, 0);
            if (wsIsConnected(clients[i]))
                ret += 1;
            else {
                clients[i].scallback(i, WEBSOCKET_DISCONNECT_ERROR, NULL, 0);
                wsDisconnectClient(&clients[i]);
            }
        }
    }
    return ret;
}


} // namespace server