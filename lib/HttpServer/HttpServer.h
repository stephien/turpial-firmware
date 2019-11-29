#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "Task.h"
#include "esp_log.h"
#include "lwip/api.h"
#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"



const static char HTML_HEADER[] = "HTTP/1.1 200 OK\nContent-type: text/html\n\n";
static QueueHandle_t client_queue;
static const int client_queue_size = 10;

class HttpServer : public Task
{
public:
    HttpServer();
    virtual ~HttpServer();
    void setTimeout(int timeout);

   // void serverTask(void* data);
    //
    virtual void run(void* data) override
    {
        const static char* TAG = "server_task";
        static err_t err;
        client_queue = xQueueCreate(client_queue_size, sizeof(struct netconn*));

        conn_ = netconn_new(NETCONN_TCP);
        netconn_bind(conn_, NULL, 8080);
        netconn_listen(conn_);
        ESP_LOGI(TAG, "server listening");
        do {
            err = netconn_accept(conn_, &newconn_);
            if (err == ERR_OK) {
                ESP_LOGI(TAG, "new client");
                xQueueSendToBack(client_queue, &newconn_, portMAX_DELAY);
                //http_serve(newconn);
            }
        } while (1);
        netconn_close(conn_);
        netconn_delete(conn_);
        ESP_LOGE(TAG, "task ending, rebooting board");
        //esp_restart();
      
    }

private:
    struct netconn* conn_;
    struct netconn* newconn_;
    struct netbuf* inbuf_;
    static char* buf_;
    static uint16_t buflen_;
    static err_t err_;
    int16_t timeout_;
    int port_ = 80;
};


/* class Hellos : public Task
{
    void run(void* data) override
    {
        char* pc_task_name = reinterpret_cast<char*>(data);

        for (int i = 10; i >= 0; i--) {
          
            printf("Restarting in %d seconds...\n", i);
              ESP_LOGI("Hola", "server listening");
            vTaskDelay(1000 / portTICK_RATE_MS);
        }
        esp_restart();
        return;
    }
}; */
#endif