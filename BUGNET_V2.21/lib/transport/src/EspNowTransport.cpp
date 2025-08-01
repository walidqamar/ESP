#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "OutputBuffer.h"
#include "EspNowTransport.h"

const int MAX_ESP_NOW_PACKET_SIZE = 250;//3C:61:05:0B:D7:3C
uint8_t broadcastAddress[2][6] =  {{0x84, 0xCC, 0xA8, 0x61, 0x13, 0x90},{0xA0, 0x76, 0x4E, 0x40, 0x3C, 0xE4}};   // MAC Address of Transmitter
/////////////////////////////////////////////////////////{0x7C, 0x9E, 0xBD, 0x44, 0xBF, 0xDC}MAC: a0:76:4e:44:33:18  A0:76:4E:44:2E:10
static const char* PMK_KEY_STR = "WF4F3K6L7?R#UQ%8";
static const char* LMK_KEY_STR = "VH5U85IJp#$78FTP";
/////////////////////////////////////////////////////////
static EspNowTransport *instance = NULL;
//static int jj=0;
void receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen)
{
 //int chnl;
  //memcpy(&chnl, data, sizeof(chnl));
  // annoyingly we can't pass an param into this so we need to do a bit of hack to access the EspNowTransport instance
  int header_size = instance->m_header_size;
  
  // first m_header_size bytes of m_buffer are the expected header
  if ((dataLen > header_size) && (dataLen<=MAX_ESP_NOW_PACKET_SIZE) && (memcmp(data,instance->m_buffer,header_size) == 0)) 
  {
    instance->m_output_buffer->add_samples(data + header_size, dataLen - header_size);
  }
}

bool EspNowTransport::begin(int i)
{
  // Set Wifi channel
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(m_wifi_channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  
  esp_err_t result = esp_now_init();
  if (result != ESP_OK)
  {
    Serial.println("ESPNow Init Failed");
   return false;
  }
  else
  {
    esp_now_register_recv_cb(receiveCallback);
  }
 // Setting the PMK key
  esp_now_set_pmk((uint8_t *)PMK_KEY_STR); 
  esp_now_peer_info_t peerInfo = {};
  memcpy(&peerInfo.peer_addr, broadcastAddress[i], 6);

  /////////////////////////////////////////////////////
  // Setting the master device LMK key
  for (uint8_t i = 0; i < 16; i++) {
   peerInfo.lmk[i] = LMK_KEY_STR[i];
  }
  peerInfo.encrypt = true;
  ///////////////////////////////////////////////////////

  if (!esp_now_is_peer_exist(broadcastAddress[i]))
  {
    result = esp_now_add_peer(&peerInfo);
    if (result != ESP_OK)
    {
      Serial.printf("Failed to add broadcast peer: %s\n", esp_err_to_name(result));
      return false;
    }
  }
  return true;
}


EspNowTransport::EspNowTransport(OutputBuffer *output_buffer, uint8_t wifi_channel) : Transport(output_buffer, MAX_ESP_NOW_PACKET_SIZE)
{
  instance = this;  
  m_wifi_channel = wifi_channel;
}

