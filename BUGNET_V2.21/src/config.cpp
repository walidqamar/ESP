#include "config.h"

// In case each transport packet needs to start with a specific header, define transport_header here.
// TRANSPORT_HEADER_SIZE needs to be defined in config.h
// For example, when TRANSPORT_HEADER_SIZE is defined as 3,  define transport_header for example as {0x1F, 0xCD, 0x01};
uint8_t transport_header[TRANSPORT_HEADER_SIZE] = {};

// i2s speaker pins
i2s_pin_config_t i2s_speaker_pins = 
{
    .bck_io_num = I2S_SPEAKER_SERIAL_CLOCK,
    .ws_io_num = I2S_SPEAKER_LEFT_RIGHT_CLOCK,
    .data_out_num = I2S_SPEAKER_SERIAL_DATA,
    .data_in_num = I2S_PIN_NO_CHANGE
};
