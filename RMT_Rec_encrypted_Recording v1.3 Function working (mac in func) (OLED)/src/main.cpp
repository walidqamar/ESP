#include <Arduino.h>
#include <driver/i2s.h>
#include <WiFi.h>
#include <esp_now.h>
#include "I2SOutput.h"
#include "EspNowTransport.h"
#include "OutputBuffer.h"
#include "config.h"
#include <SDCard.h>
#include <WAVFileWriter.h>
#include <RecordNewFile.h>
#include <SPI.h>
#include <U8g2lib.h>

uint8_t macAddress[] ={0x64, 0xE8, 0x33, 0x85, 0x1C, 0xBC};
int sample_counter, counter=0, count=0, initial_val=0, channel=1;
bool rec_check=false;
unsigned long lastReceivedTime = 0;
const unsigned long receiveInterval = 10000;
unsigned long currentTime = millis();
U8G2_SSD1322_ZJY_256X64_F_4W_SW_SPI u8g2(U8G2_R0, 17, 16, 32, 21, 4);
bool first_screen=true, pressed_button = false, second_screen=false, third_screen=false,welcome=true;
int SCROLL_A=0, SCROLL_B=0, SCROLL_C=0, init_channel=1;
const gpio_num_t keyPins[4] = { GPIO_NUM_27, GPIO_NUM_25, GPIO_NUM_26, GPIO_NUM_35}; 
const gpio_num_t RECORD = GPIO_NUM_12;
const char* keyNames[4] = { "Back", "Up", "Down", "Select" };
enum Screen { WELCOME, MENU1, MENU2, MENU3, MENU4, MENU5};
enum StateButton {CLICKED, IDLE}; 
StateButton statebutton = IDLE;
Screen screen = WELCOME;
uint8_t selectedOption = 0;
const char* menuOptions[4] = {
    "BUG 1",
    "BUG 2",
    "BUG 3",
    "BUG 4"
};
const uint8_t NUM_OPTIONS = 4;
void welcome_screen(const char* title)
  {
    u8g2.setFont(u8g2_font_ncenB14_tr);
    int16_t w = u8g2.getStrWidth(title);
    u8g2.drawStr((256 - w) / 2, (64 - 20) / 2, title);
    u8g2.setFont(u8g2_font_ncenB10_tf);
    //u8g2.setFont(u8g2_font_unifont_tr);
    u8g2.drawHLine(0, 40, 256);
    u8g2.drawStr(15, 45, "Audio BUG-NET Dte of AI TECH");
    u8g2.sendBuffer();
    delay(5000);
    Serial.println("WELCOME SCREEN");
  }
void drawBatteryAndRSSI(int batteryPercentage, int rssi, const char* title)
  {
    // Battery drawing at top-right
    int battWidth = 20;
    int battHeight = 8;
    int x = 256 - battWidth - 2;
    int y = 2;
    u8g2.drawFrame(x, y, battWidth, battHeight);
    u8g2.drawBox(x + battWidth, y + 2, 2, 4);
    batteryPercentage = constrain(batteryPercentage, 0, 100);
    int levelWidth = (battWidth - 4) * batteryPercentage / 100;
    u8g2.drawBox(x + 2, y + 2, levelWidth, battHeight - 4);
    int baseY = 11;
    int barWidth = 2;
    int spacing = 1;
    int heights[4] = {3, 5, 7, 9};
    int bars = (rssi > -60) ? 4 : (rssi > -70) ? 3 : (rssi > -80) ? 2 : (rssi > -90) ? 1 : 0;
    Serial.println("RSSI: " + String(rssi) + " Bars: " + String(bars));
    int startX = 10;
    for (int i = 0; i < bars; i++) 
    {
      int bx = startX + i * (barWidth + spacing);
      int by = baseY - heights[i];
      if (i < bars) 
      {
        u8g2.drawBox(bx, by, barWidth, heights[i]);
      } else 
      {
        u8g2.drawFrame(bx, by, barWidth, heights[i]);
      }
    }
    // Title and decoration
    int W = u8g2.getDisplayWidth();
    int fh = 12;
    int f_pos = 100;
    u8g2.drawStr(f_pos, 0, title);
    u8g2.setFont(u8g2_font_unifont_t_symbols);
    u8g2.setFontPosTop();
    u8g2.drawUTF8(1, 1, "↗");
    u8g2.sendBuffer();
  }
int send_channel(int &initial_val, int &channel)
  {
      Serial.println(String("Initial Value in Send channel Function: ") + initial_val);
      if (initial_val == 0)
      {
          esp_err_t result = esp_now_send(macAddress, (uint8_t *) &channel, sizeof(channel));
          if (result != ESP_OK)
          {
              Serial.printf("Failed to send: %s\n", esp_err_to_name(result));
          }
          Serial.println(String("Send channel function.. : ") + channel);
      }
      return initial_val;
  }
int send_reset_channel(int &initial_val, int &Reset_channel)
  {
      esp_err_t result = esp_now_send(macAddress, (uint8_t *) &Reset_channel, sizeof(Reset_channel));
      if (result != ESP_OK)
        {
          Serial.printf("Failed to send: %s\n", esp_err_to_name(result));
        }
      Serial.println(String("RESET CHANNEL  in RESET function Called : ") + Reset_channel);
      Serial.println(String("Initial Value in RESET Function: ") + initial_val);
      return initial_val;
  }
void wait_until_stopButton_push()
  {
    while (gpio_get_level(GPIO_Stop_Button) == 0)
      {
        vTaskDelay(pdMS_TO_TICKS(100));
      }
    count=1;
  }
void setup()
  {
    Serial.begin(115200);
    setCpuFrequencyMhz(80);  
    Serial.print("Receiver MAC Address:  ");
    Serial.println(WiFi.macAddress());
    pinMode(Recording_Start_Stop_Button , INPUT_PULLDOWN);  
    while (!Serial) { /* wait for the USB serial to be ready */ }
    u8g2.begin();
    u8g2.enableUTF8Print();            // ← add this
    u8g2.setFontPosTop();
    for (int i = 0; i < 4; i++) 
      {
        pinMode(keyPins[i], INPUT_PULLDOWN);
      }
  }
void loop()
{
  pinMode(GPIO_Stop_Button, INPUT_PULLDOWN);
  new SDCard("/sdcard", PIN_NUM_MISO, PIN_NUM_MOSI, PIN_NUM_CLK, PIN_NUM_CS);
  OutputBuffer *m_output_buffer = new OutputBuffer(300 * 16);
  Output *m_output = new I2SOutput(I2S_NUM_0, i2s_speaker_pins);
  Transport *m_transport;
  m_output->start(SAMPLE_RATE);
  A: 
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  const char* title = "SELECT BUG";
  if(screen==WELCOME)
    { 
      const char *msg = "BUGNET";
      drawBatteryAndRSSI(0, -75," ");  // Example call: 65% battery, -75 dBm RSSI
      welcome_screen(msg);
    }
  screen=MENU1;
  drawBatteryAndRSSI(0, -65," ");
  m_transport = new EspNowTransport(m_output_buffer,channel);
  m_transport->set_header(TRANSPORT_HEADER_SIZE,transport_header);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  m_transport->begin();
  esp_err_t result = esp_now_init();
  if (result != ESP_OK)
    {
      Serial.println("ESPNow Init Failed");
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  wait_until_stopButton_push();
  Serial.println("After waits");
  initial_val = send_channel(initial_val, channel);
  if(initial_val==0)
    {
      initial_val=1;
      delete m_transport;
      goto A;
    }
  Serial.print("Channel number : ");
  Serial.print(channel);
  Serial.println("  Sent" );
  int16_t *samples = reinterpret_cast<int16_t *>(malloc(sizeof(int16_t) * 128));
  while (gpio_get_level(GPIO_Stop_Button)==1)
  {
    while (gpio_get_level(Recording_Start_Stop_Button)==0 && gpio_get_level(GPIO_Stop_Button)==1)
      {
        currentTime = millis();
        if (currentTime - lastReceivedTime >= receiveInterval)
          {
            vTaskDelay(pdMS_TO_TICKS(500));
            Serial.println("No data received in the last 10 seconds");
            vTaskDelay(pdMS_TO_TICKS(500));
          }
        m_output_buffer->remove_samples(samples, 128);
        m_output->write(samples, 128);
      }
    RecordNewFile rf;  
    uint32_t filesize=0;
    rf.writeFile();
    FILE *fp = fopen(rf.sdarr, "wb");
    WAVFileWriter *writerFile = new WAVFileWriter(fp, 16000);
    while (gpio_get_level(Recording_Start_Stop_Button)==1 && gpio_get_level(GPIO_Stop_Button)==1)
      {    RecordNewFile rf;  
    uint32_t filesize=0;
    rf.writeFile();
    FILE *fp = fopen(rf.sdarr, "wb");
    WAVFileWriter *writerFile = new WAVFileWriter(fp, 16000);
        currentTime = millis();
        if (currentTime - lastReceivedTime >= receiveInterval)
          {
            vTaskDelay(pdMS_TO_TICKS(500));
            Serial.println("No data received in the last 10 seconds");
            vTaskDelay(pdMS_TO_TICKS(500));
          }
        m_output_buffer->remove_samples(samples, 128);
        m_output->write(samples, 128);
        sample_counter=0;  
        filesize=writerFile->write(samples, 128);
        if(filesize>max_filesize)
          {
            break;                   // Check the file Size during recording
          }
      }
    writerFile->finish();
    fclose(fp);
    int RESET=16;
    initial_val = send_reset_channel(initial_val, RESET);
    initial_val=0;
    delete m_transport;
    goto A;
  }
  // nothing to do - the application is doing all the work
  vTaskDelay(pdMS_TO_TICKS(1000));
}

 



