
#include "hal/hardware.h"
#include <stdio.h>
#include <iostream>

#include "WiFiDTO.h"
#include "SerializablePOD.h"

//#include "Serializable.h"



//typedef struct wifi_dto_config_t

//constructor 
WiFiDTO::WiFiDTO(wifi_dto_config_t& settings) {
  settings_.WAP_enabled = settings.WAP_enabled;
  settings_.apChannel = settings.apChannel;
  settings_.apMaxConn = settings.apMaxConn;
  settings_.apPassword = settings.apPassword;
  settings_.apSSID = settings.apSSID;
  settings_.WAP_enabled = settings.WAP_enabled;
  settings_.WST_enabled =settings.WST_enabled;
  // settings_ = settings;
  std::cout<<"esto es una prueba mas"<<std::endl;
  std::cout<<"constructor working fine"<<std::endl;

  std::cout << "Showing Data" << std::endl;
  std::cout << "Settings.apChannel " << settings_.apChannel << std::endl;
  std::cout << "Settings.apChannel " << settings_.apMaxConn << std::endl;
  std::cout << "Settings.apPassword " << settings_.apPassword << std::endl;
  std::cout << "Settings.SSID " << settings_.apSSID << std::endl;
  std::cout << "Settings.Wap Enabled " << settings_.WAP_enabled << std::endl;
  /*std::cout << "Settings.WST" << settings_.WST_enabled << std::endl;   */
} 
//default constructor
WiFiDTO::WiFiDTO() {
  std::cout <<"contructor de void*" <<std::endl;
 }
//implementation of all virtual methods from serializable interface

size_t WiFiDTO::serialize_size() const {
  size_t size;  
  size =  SerializablePOD<bool>::serialize_size(settings_.WAP_enabled) +
          SerializablePOD<bool>::serialize_size(settings_.WST_enabled) +
          SerializablePOD<bool>::serialize_size(settings_.isOpen) +
          SerializablePOD<char*>::serialize_size(settings_.apSSID)  + 
          SerializablePOD<char*>::serialize_size(settings_.apPassword);
         /*  SerializablePOD<int32>::serialize_size(settings_.apChannel) +   
          SerializablePOD<int32>::serialize_size(settings_.apMaxConn);  */
                     
  return size; 
}

void WiFiDTO::serialize(char* dataOut) const {
  
  std::cout <<"Wap Enabled: bool------------->"<< static_cast<const void*>(dataOut)<<std::endl;
  dataOut = SerializablePOD<bool>::serialize(dataOut, settings_.WAP_enabled);
  dataOut = dataOut + sizeof(settings_.WAP_enabled);
  
  std::cout <<"WST_enabled(bool): ---------->"<< static_cast<const void*>(dataOut)<<std::endl;
  SerializablePOD<bool>::serialize(dataOut, settings_.WST_enabled);
  dataOut += sizeof(settings_.WST_enabled);

  std::cout <<"isOpen: (bool)------------->"<< static_cast<const void*>(dataOut)<<std::endl;
  SerializablePOD<bool>::serialize(dataOut, settings_.isOpen);
  dataOut +=sizeof(bool);
  
  std::cout <<"apSSID: (char*) --------->"<< static_cast<const void*>(dataOut)<<std::endl;
  SerializablePOD<char*>::serialize(dataOut, settings_.apSSID);
  dataOut += strlen(settings_.apSSID);

  std::cout <<"apPassword: (char*) -------->"<< static_cast<const void*>(dataOut)<<std::endl;
  SerializablePOD<char*>::serialize(dataOut, settings_.apPassword);
  dataOut += strlen(settings_.apPassword); 
 
  //missin int32 type


} 

void WiFiDTO::deserialize(const char* dataIn) { 

} 


