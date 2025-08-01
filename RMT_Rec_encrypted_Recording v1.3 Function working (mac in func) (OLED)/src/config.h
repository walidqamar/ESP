#include <freertos/FreeRTOS.h>
#include <driver/i2s.h>
#include <driver/gpio.h>

// sdcard                                       ESP-WROOM-32
#define PIN_NUM_MISO GPIO_NUM_2 //19   (VSPI (MISO, CLK, MOSI, CS)=(19, 18, 23, 5))
#define PIN_NUM_CLK GPIO_NUM_14  //18   (HSPI (MISO, CLK, MOSI, CS)=(12, 14, 13, 15))
#define PIN_NUM_MOSI GPIO_NUM_15  //23   TTGO SD Card pin (MISO, CLK, MOSI, CS)=(2, 14, 15, 13)
#define PIN_NUM_CS GPIO_NUM_13    //25
// sample rate for the system
#define SAMPLE_RATE 16000
//set file size
const uint32_t max_filesize=9677000;//0; 

// ==========speaker settings===========
#define I2S_SPEAKER_SERIAL_CLOCK GPIO_NUM_18        //16 BCLK
#define I2S_SPEAKER_LEFT_RIGHT_CLOCK GPIO_NUM_19   //17  LRC
#define I2S_SPEAKER_SERIAL_DATA GPIO_NUM_5        //15   DIN
// GAIN-------GND

//#define I2S_SPEAKER_SD_PIN GPIO_NUM_22
// configurations button
#define GPIO_Stop_Button GPIO_NUM_12//GPIO_NUM_23
#define Recording_Start_Stop_Button GPIO_NUM_22//GPIO_NUM_25
///#define Channel_Increment_Button GPIO_NUM_26
////#define Channel_Decrement_Button GPIO_NUM_33
///#define Select_Channel_Button GPIO_NUM_27

// In case all transport packets need a header (to avoid interference with other applications or walkie talkie sets), 
// specify TRANSPORT_HEADER_SIZE (the length in bytes of the header) in the next line, and define the transport header in config.cpp
#define TRANSPORT_HEADER_SIZE 0
extern uint8_t transport_header[TRANSPORT_HEADER_SIZE];

// i2s speaker pins
extern i2s_pin_config_t i2s_speaker_pins;

