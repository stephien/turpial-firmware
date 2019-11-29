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
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
//#include "Task.h"

#include "NVS.h"
#include "WiFiMode.h"

#include "WiFiEventHandler.h"

#include "defaults.h"

#include "WebSocket_Task.h"

#include "HttpServer.h"
#include "testRTOSCPP/Hello.h"
static const char* TAG = "app_main";
//WebSocket frame receive queue
QueueHandle_t WebSocket_rx_queue;

void task_process_WebSocket( void *pvParameters ){
    (void)pvParameters;

    //frame buffer
    WebSocket_frame_t __RX_frame;

    //create WebSocket RX Queue
    WebSocket_rx_queue = xQueueCreate(10,sizeof(WebSocket_frame_t));

    while (1){
        //receive next WebSocket frame from queue
        if(xQueueReceive(WebSocket_rx_queue,&__RX_frame, 3*portTICK_PERIOD_MS)==pdTRUE){

        	//write frame inforamtion to UART
        	printf("New Websocket frame. Length %d, payload %.*s \r\n", __RX_frame.payload_length, __RX_frame.payload_length, __RX_frame.payload);

        	//loop back frame
        	WS_write_data(__RX_frame.payload, __RX_frame.payload_length);

        	//free memory
			if (__RX_frame.payload != NULL)
				free(__RX_frame.payload);

        }
    }
}



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


static HttpServer *httpServer;
/* 
static void http_task(void* params) {
char* data = "heloWorld";
httpServer->serverTask((void*)data);
} */

extern "C" void app_main()
{
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
    vTaskDelay(1000 / portTICK_RATE_MS);

    httpServer = new HttpServer();
    httpServer->start();
   
    //httpServer->setStackSize(4000);
    //httpServer->start((void*)pcTask1); */
   // httpServer->start((void*)pcTask1);

}