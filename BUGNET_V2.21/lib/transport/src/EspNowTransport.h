#pragma once

#include "Transport.h"

class OutputBuffer;

class EspNowTransport: public Transport 
{
 private:
  uint8_t m_wifi_channel;

 public:
  EspNowTransport(OutputBuffer *output_buffer, uint8_t wifi_channel);
  virtual bool begin(int i) override;
  friend void receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen);
};
