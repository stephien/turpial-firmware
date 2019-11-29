#include "HttpServer.h"


#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"


HttpServer::HttpServer()
{
    client_queue = xQueueCreate(client_queue_size, sizeof(struct netconn*));
    conn_ = new netconn();
    netconn_bind(conn_, NULL, port_);
}


HttpServer::~HttpServer()
{
}


/* void HttpServer::start()
{
    netconn_listen(conn_);
    ESP_LOGI(TAG, "server listening");


    do {
        err_ = netconn_accept(conn_, &newconn_);
        if (err_ == ERR_OK) {
            xQueueSendToBack(client_queue, &newconn_, portMAX_DELAY);
            //http_serve(newconn_)
        }
    } while (err_ == ESP_OK);
    netconn_close(conn_);
    netconn_delete(conn_);
    ESP_LOGE(TAG, "task ending, rebooting board");
    esp_restart();
}
 */
