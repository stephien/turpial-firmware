/**
 * @file main.cpp
 * @author Locha Mesh Developers (contact@locha.io)
 * @brief 
 * @version 0.1
 * @date 2019-09-11
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include <cstdio>
#include <sstream>

#include "esp_log.h"
#include "sdkconfig.h"
/* #include <freertos/FreeRTOS.h>
#include <freertos/task.h> */


#include "NVS.h"
#include "WiFiMode.h"

#include "WiFiEventHandler.h"

#include "defaults.h"
#include "TcpServer.h"
//#include "WebSocketServer.h"


/* #include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#define MAX 255
#define PORT 80
#define SA struct sockaddr */


static const char* TAG = "app_main";
//WebSocket frame receive queue
//QueueHandle_t WebSocket_rx_queue;

esp_err_t getIsConfigured(bool& is_configured)
{
    esp_err_t err;

    storage::NVS app_nvs;
    err = app_nvs.open(NVS_APP_NAMESPACE, NVS_READWRITE);
    if (err != ESP_OK) {
        const char* err_str = esp_err_to_name(err);
        ESP_LOGE(TAG,
            "Couldn't open namespace \"%s\" (%s)",
            NVS_APP_NAMESPACE,
            err_str);
        return err;
    }

    err = app_nvs.get_bool(NVS_IS_CONFIGURED_KEY, is_configured);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        // Set is_configured to true on flash so on next init the config is
        // readed directly by the ESP-IDF Wi-Fi library component.
        err = app_nvs.set_bool(NVS_IS_CONFIGURED_KEY, true);
        if (err != ESP_OK) return err;
        err = app_nvs.commit();
        if (err != ESP_OK) return err;
        // Set the return variable to "false" to forcibly set the default
        // configuration
        is_configured = false;
    } else {
        return err;
    }

    return ESP_OK;
}


// Function designed for chat between client and server.
void func(int sockfd)
{
    char buff[MAX];
    int n;
    for (;;) {
        bzero(buff, MAX);
        // read the message from client and copy it in buffer
        read(sockfd, buff, sizeof(buff));
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
    }
}


extern "C" void app_main()
{ //static server::WebSocketServer *server;
    static server::TcpServer *tcpServer;
    esp_err_t err;
    wifi::WiFiEventHandler* event_handler;
    event_handler = new wifi::WiFiEventHandler();
    wifi::WiFiMode* wifi_mode;
    wifi_mode = new wifi::WiFiMode();


    bool is_nvs_initialized = true;
    err = storage::init();
    if (err != ESP_OK) {
        const char* err_name = esp_err_to_name(err);
        ESP_LOGE(TAG, "Couldn't initialize NVS, error (%s)", err_name);
        is_nvs_initialized = false;
    }

    ESP_LOGD(TAG, "Init TCP/IP adapter");
    tcpip_adapter_init();

    bool is_configured = false;
    if (is_nvs_initialized) {
        err = getIsConfigured(is_configured);
        if (err != ESP_OK) {
            const char* err_str = esp_err_to_name(err);
            ESP_LOGE(TAG,
                "Couldn't get \"is_configured\" value (%s)",
                err_str);
        }
    }

    err = wifi_mode->init(is_nvs_initialized);
    if (err != ESP_OK) {
        const char* err_name = esp_err_to_name(err);
        ESP_LOGE(TAG, "Couldn't initalize Wi-Fi interface (%s)", err_name);
        // TODO: fallback to bluetooth mode to configure Wi-Fi?
        return;
    }

    if (!is_configured) {
        wifi_mode->set_mode(WIFI_MODE);

        wifi::APConfig ap_config = {
            .ssid = WAP_SSID,
            .password = WAP_PASS,
            .authmode = WAP_AUTHMODE,
            .max_conn = WAP_MAXCONN,
            .channel = WAP_CHANNEL,
        };
        wifi_mode->set_ap_config(ap_config);

        wifi::STAConfig sta_config = {
            .ssid = WST_SSID,
            .password = WST_PASS,
        };
        wifi_mode->set_sta_config(sta_config);
    }

    wifi_mode->setWiFiEventHandler(event_handler);
    err = wifi_mode->start();


    // TODO: app loop

      tcpServer = new server::TcpServer();
    tcpServer->start(); 

}
    // server = new server::WebSocketServer();
    // server->start();
    // server->ws_server_start();

//server here /////////////////////////////////
   /*  int sockfd, connfd;
    socklen_t len;
    struct sockaddr_in servaddr, cli;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        ESP_LOGI("*****************", "socket creation failed...\n");
        exit(0);
    } else
        ESP_LOGI("*****************", "Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        ESP_LOGI("******************", "socket bind failed...\n");
        exit(0);
    } else
        ESP_LOGI("**************************", "Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
        ESP_LOGI("*****************************", "Listen failed...\n");
        exit(0);
    } else
        ESP_LOGI("*****************************", "Server listening..\n");
    len = sizeof(cli);


    // Accept the data packet from client and verification
    connfd = accept(sockfd, (SA*)&cli, &len);
    if (connfd < 0) {
        printf("server acccept failed...\n");
        exit(0);
    } else {
        printf("server acccept the client...\n");
     ESP_LOGI("VALUE OF SOCKET FILE DESCRIPTOR CONECTION:   ", "Value: %d", connfd);
    }
    // Function for chatting between client and server

    func(connfd);
} */