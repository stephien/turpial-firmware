#include "HttpServer.h"
#include <string.h>
#include <errno.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

namespace server {
HttpServer::HttpServer()
{
    client_queue = xQueueCreate(client_queue_size, sizeof(struct netconn*));
    conn_ = new netconn();
    netconn_bind(conn_, NULL, port_);
}


HttpServer::~HttpServer()
{
}

void HttpServer::setPort(int port)
{
    this->port_ = port;
}

void HttpServer::setTimeout(int16_t time)
{
    this->timeout_ = time;
}

// receives clients from queue, handles them
void HttpServer::handleClientsTask(void* pvParameters) {
  const static char* TAG = "server_handle_task";
  struct netconn* conn;
  ESP_LOGI(TAG,"task starting");
  for(;;) {
    xQueueReceive(client_queue,&conn,portMAX_DELAY);
    if(!conn) continue;
     handleHttpConnections(conn);
  }
  vTaskDelete(NULL);
}


 
// serves any clients
void HttpServer::handleHttpConnections(struct netconn *conn) {
  const static char* TAG = "http_server";
  const static char HTML_HEADER[] = "HTTP/1.1 200 OK\nContent-type: text/html\n\n";

  struct netbuf* inbuf;
  static char* buf;
  static uint16_t buflen;
  static err_t err;


  netconn_set_recvtimeout(conn,1000); // allow a connection timeout of 1 second
  ESP_LOGI(TAG,"reading from client...");
  err = netconn_recv(conn, &inbuf);
  ESP_LOGI(TAG,"read from client");
  if(err==ERR_OK) {
    netbuf_data(inbuf, (void**)&buf, &buflen);
    if(buf) {

      // default page
      if (strstr(buf,"GET / ") && !strstr(buf,"Upgrade: websocket")) {
        ESP_LOGI(TAG,"Sending /");
        netconn_write(conn, HTML_HEADER, sizeof(HTML_HEADER)-1,NETCONN_NOCOPY);
        netconn_write(conn, "Hola",4,NETCONN_NOCOPY);
        netconn_close(conn);
        netconn_delete(conn);
        netbuf_delete(inbuf);
      }

      // default page websocket
      else if(strstr(buf,"GET / ") && strstr(buf,"Upgrade: websocket")) {
        ESP_LOGI(TAG,"Requesting websocket on /");
        //ws_server_add_client(conn,buf,buflen,"/", websocket_callback);
        netbuf_delete(inbuf);
      } else {//esto se hizo para que funcione el wensocket javascript solamente
         //ws_server_add_client(conn,buf,buflen,"/", websocket_callback);
        netbuf_delete(inbuf);
        
        // ESP_LOGI(TAG,"Unknown request");
        // netconn_close(conn);
        // netconn_delete(conn);
        // netbuf_delete(inbuf);
      }
    }
    else {
      ESP_LOGI(TAG,"Unknown request (empty?...)");
      netconn_close(conn);
      netconn_delete(conn);
      netbuf_delete(inbuf);
    }
  }
  else { // if err==ERR_OK
    ESP_LOGI(TAG,"error on read, closing connection");
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

}