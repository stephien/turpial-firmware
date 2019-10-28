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

#include <Arduino.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "hal/hardware.h"
#include "WiFi/WiFiMode.h"
#include "ESC/battery.h"
#include  "WiFiDTO.h"
#include "NVS/SingletonNVS.h"


#include <iostream>
#include <string>
#include <vector>

// Creating instances of the classes
Battery battery(BATTERY_CAPACITY, LOW_BAT_THRESHOLD, CRITICAL_BAT_THRESHOLD);

//singleton instance for all the application
SingletonNVS* nvs = SingletonNVS::getInstance(); //create or recovery SingletonNVS instance as needed
WiFiMode wlan;

esp_err_t status;

esp_err_t batteryTest() {
    // Put the code here for battery test
    //esp_err_t err;
    //Battery battery(BATTERY_CAPACITY, LOW_BAT_THRESHOLD, CRITICAL_BAT_THRESHOLD);

    status = battery.begin();
    if (status != ESP_OK)
    {
        //esp_restart();
        ESP_LOGE(__func__, "Error starting battery IC module");
    }

    ESP_LOGD(__func__, "Battery level: %i%", battery.getBatteryLevel());
    ESP_LOGD(__func__, "Battery voltage: %imV", battery.getBatteryVoltage());
    ESP_LOGD(__func__, "Battery current: %iA", battery.getBatteryCurrent());
}

/*
esp_err_t bleTest() {
    // Put the code here for BLE test
    esp_err_t err;
}
*/

esp_err_t wifiTest() {
    // Put the code here for Wi-Fi test
    //esp_err_t err;
    //WiFiMode wlan;
    status = wlan.begin();
    if (status != ESP_OK)
    {
        //esp_restart();
        ESP_LOGE(__func__, "Error starting WiFi modules");
    }
}


void nvsTest() {
    // Initialize the NVS flash storage
    nvs->begin();
    // open nvs
    bool isOpen = nvs->open("namespace", false);

    if (isOpen)
    {
        ESP_LOGD(__func__, "nvs is open");
        // Save chars into the NVS
        nvs->setString(NVS_STR_KEY, "ESTA ES UNA PRUEBA CON UN STRING LARGO... SALUD!");
        //size_t str_saved = nvs.setString(NVS_STR_KEY, "ESTA ES UNA PRUEBA CON UN STRING LARGO... SALUD!");
        //ESP_LOGD(__func__, "saved %d bytes", str_saved);

        // Read chars from the NVS
        char *readString = nvs->getString(NVS_STR_KEY, "ERROR");

        if (readString != "ERROR" && readString)
        {
            ESP_LOGD(__func__, "have a key w/value %s", readString);
            free(readString);
        }
        else
        {
            ESP_LOGE(__func__, "error reading value w/key");
        }

        // Save int value into the NVS
        nvs->setInt(NVS_INT_KEY, 23987);
        // Read int from the NVS
        int32_t readInt = nvs->getInt(NVS_INT_KEY, 0);
        ESP_LOGD(__func__, "Int value from NVS: %i", readInt);

        // Save bool value into the NVS
        nvs->setBool(NVS_BOOL_KEY, 1);
        // Read int from the NVS
        bool readBool = nvs->getBool(NVS_BOOL_KEY, 0);
        ESP_LOGD(__func__, "Bool value from NVS: %i", readBool);
    }
    else
    {
        ESP_LOGE(__func__, "Error opening the NVS");
    }
}


void checkForCriticalLevels(){
    if (battery.isBatteryCritical()) 
    {
        // Power off the device
    }
}



void setup() {
   
   /*  nvs->setValue(10);
    SingletonNVS* p2 = SingletonNVS::getInstance();
    p2->setValue(150);
    std::cout<<"value = "<<nvs->getValue() << std::endl; */
 
    //initial test object
    wifi_dto_config_t wifi_params;  
    size_t length;
    char* buffer;
    //to interpolate information relate with wifi data stored
    //dta just for test serialized functionalities
    wifi_params.WAP_enabled = WAP_ENABLED; // Default value
    wifi_params.WST_enabled = WST_ENABLED; // Default value
    wifi_params.isOpen = true;
    wifi_params.apSSID = "hello world";
    wifi_params.apPassword = "mi password largo";
    wifi_params.apChannel = WAP_CHANNEL;
    wifi_params.apMaxConn = WAP_MAXCONN;
    
    
    WiFiDTO wifi_dto(wifi_params); //object to be serialized
    length = wifi_dto.serialize_size(); //get the length of the dto class
    
    //buffer to store the serialized data
    buffer = (char*)malloc(sizeof(char)*length);
    wifi_dto.serialize(buffer);

   // wifi_dto.deserialize(buffer);

   std::cout<<"The length is :" << length << std::endl; 
   /*
    // Initialize battery module
    status = battery.begin();
    if (status != ESP_OK)
    {
        //esp_restart();
        ESP_LOGE(__func__, "Error starting battery IC module!");
    }
    // Check for critical levels
    checkForCriticalLevels();
    */
    // Initialize Non-Volatile Storage
    /* status = nvs->begin();
    if (status != ESP_OK)
    {
        //esp_restart();
        ESP_LOGE(__func__, "Error starting NVS!");
    }  */

    // Initialize BLE
    // Put the code here...


    // Initialize Wi-Fi module
    /* status = wlan.begin();
    if (status != ESP_OK)
    {
        //esp_restart();
        ESP_LOGE(__func__, "Error starting WiFi modules!");
    } */

    // Initialize Radio module
    // Put the code here...
}

void loop()
{
    //
}