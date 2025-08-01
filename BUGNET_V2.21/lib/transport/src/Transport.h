#pragma once
#include <stdlib.h>
#include <stdint.h>

class OutputBuffer;

class Transport
{
 protected:
  // audio buffer for samples we need to send
  uint8_t *m_buffer = NULL;
  int m_buffer_size = 0;
  int m_index = 0;
  int m_header_size;
  OutputBuffer *m_output_buffer = NULL;

 public:
  Transport(OutputBuffer *output_buffer, size_t buffer_size);
  int set_header(const int header_size, const uint8_t *header);
  virtual ~Transport();
  int ack_function(int a);
  virtual bool begin(int i) = 0;
};
