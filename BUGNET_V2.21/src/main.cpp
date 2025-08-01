#include <Arduino.h>
#include <driver/i2s.h>
#include <WiFi.h>
#include <esp_now.h>
#include "I2SOutput.h"
#include "EspNowTransport.h"
#include "OutputBuffer.h"
#include "config.h"
#include "SDCard.h"
#include <WAVFileWriter.h>
#include <RecordNewFile.h>
#include <SPI.h>
#include <U8g2lib.h>

Transport *m_transport;
U8G2_SSD1322_ZJY_256X64_F_4W_SW_SPI u8g2(U8G2_R0, 17, 16, 32, 21, 4);
bool first_screen=true, pressed_button = false, second_screen=false, third_screen=false,welcome=true;
uint8_t macAddress[2][6] = {{0x84, 0xCC, 0xA8, 0x61, 0x13, 0x90},{0xA0, 0x76, 0x4E, 0x40, 0x3C, 0xE4}};
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
void Navigate_Screen(const char* title);
void Menu_Screen(const char* title, const char* options[], uint8_t numoptions, uint8_t highlightIndex);
void Channel_Screen(const char* title, const char* Channel);  /* draw title, number & arrows */
void waitForPress(gpio_num_t pin);
void setupEspNowTransport(int init_channel, OutputBuffer* m_output_buffer);

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
void setup() 
  {
    Serial.begin(115200);
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
    //
    OutputBuffer *m_output_buffer = new OutputBuffer(300 * 16);
    Output *m_output = new I2SOutput(I2S_NUM_0, i2s_speaker_pins);
    m_output->start(SAMPLE_RATE);
    setupEspNowTransport(init_channel,m_output_buffer);
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB14_tr);
    const char* title = "SELECT BUG";
    if(screen==WELCOME)
      { 
        const char *msg = "BUGNET";
        drawBatteryAndRSSI(65, -75," ");  // Example call: 65% battery, -75 dBm RSSI
        welcome_screen(msg);

      }
      screen=MENU1;
      drawBatteryAndRSSI(95, -65," ");
    while (gpio_get_level(keyPins[3]) == IDLE || pressed_button == false) // keypad[3] = Select,// keypad[2] = Down, // keypad[1] = Up,// keypad[0] = Back,
      {
        if(screen==MENU1)
          {
            Serial.println("First Screen - Button not Pressed");
            u8g2.clearBuffer();
            
            Menu_Screen(title,menuOptions,NUM_OPTIONS,6); //RANDOM VALUE
            drawBatteryAndRSSI(65, -45," ");
          }
        screen=MENU2;
        delay(200);
        if (gpio_get_level(keyPins[3]) == CLICKED)
        {
          u8g2.clearBuffer();
          
          Menu_Screen(title,menuOptions,NUM_OPTIONS,SCROLL_A); //RANDOM VALUE
          drawBatteryAndRSSI(65, -45," ");
          pressed_button=true;
          Serial.println(" NEXT PRESSED - NEXT IS BUG SELECTION");
          break; 
        }
        if (gpio_get_level(keyPins[1]) == CLICKED || gpio_get_level(keyPins[2]) == CLICKED )
          {
            
            if (gpio_get_level(keyPins[2]) == CLICKED)
              {
               SCROLL_A < 3 && SCROLL_A++;
               Serial.println("SCROLLED BUG DOWN");
              }
            else if ((gpio_get_level(keyPins[1]) == CLICKED))
              {
                Serial.println("SCROLLED BUG UP");
                SCROLL_A && SCROLL_A--;
              }
            if (screen==MENU2)
              {
                Menu_Screen(title,menuOptions,NUM_OPTIONS,SCROLL_A);
                pressed_button=true;
               Serial.println("BUG SELECTION SCREEN");
              }
            vTaskDelay(pdMS_TO_TICKS(20));
          }
        //Serial.println("No UP or Down Pressed ");
      }
    pressed_button=false;
    Serial.println("BUG SELECTION Exited  | SELECT is pressed | Channel is Next");
    while (gpio_get_level(keyPins[3]) == IDLE || pressed_button == false)
      {
        screen=MENU3;
        Serial.println("Channel selection screen");
        if(screen==MENU3)
          {
            char channelBuf[6]; 
            char titleBuf[26];  // 17 chars + 1 digit + '\0'
            snprintf(titleBuf,sizeof(titleBuf),"BUG : %u  | SELECT CHANNEL",SCROLL_A+1);
            snprintf(channelBuf, sizeof(channelBuf),"%u", SCROLL_B);
            Channel_Screen(titleBuf, channelBuf); 
            Serial.println("Channel selection screen | if menu3");
          }
        if(gpio_get_level(keyPins[3]) == CLICKED)
        {

          pressed_button == true;
          Serial.println(" NEXT PRESSED - NEXT IS LIVE SCREEN");
          break; 
        }
        if (gpio_get_level(keyPins[1]) == CLICKED || gpio_get_level(keyPins[2]) == CLICKED )
          {
            screen=MENU4;
            if (gpio_get_level(keyPins[2]) == CLICKED)
              { SCROLL_B < 11 && SCROLL_B++; Serial.println("CHANNEL DOWN PRESSED"); }
            else if ((gpio_get_level(keyPins[1]) == CLICKED))
              {
                SCROLL_B && SCROLL_B--;Serial.println("CHANNEL DOWN PRESSED");
              }
            if(screen==MENU4)
              {
                char channelBuf[6]; 
                char titleBuf[26];  // 17 chars + 1 digit + '\0'
                snprintf(titleBuf,sizeof(titleBuf),"BUG : %u  | SELECT CHANNEL",SCROLL_A+1);
                snprintf(channelBuf, sizeof(channelBuf), "%u", SCROLL_B);
                Channel_Screen(titleBuf, channelBuf); 
                pressed_button == true;
                Serial.println("CHANNEL SELECT MENU");
              }
          }
        
      }
    pressed_button == false;
    Serial.println("While Exited Channel SELECT is pressed");
    while (gpio_get_level(keyPins[3]) == IDLE )//|| pressed_button == false)
      {
        u8g2.clearBuffer();
        screen=MENU5;
        Serial.println("MENU5 entered");
        if (screen==MENU5)
          {
            Serial.println("MENU5");
            u8g2.clearBuffer();
            char titleBuf[26];  // 17 chars + 1 digit + '\0'
            snprintf(titleBuf,sizeof(titleBuf),"BUG : %u  | CHANNEL : %u",SCROLL_A+1, SCROLL_B);
            Navigate_Screen(titleBuf);
          }
      }
    pressed_button == false;
      esp_err_t result = esp_now_send(macAddress[1], (uint8_t *) &SCROLL_B,sizeof(SCROLL_B));
        if (result != ESP_OK) 
          {
            Serial.println("Failed to send data");
          }
      delete m_transport;
      Serial.print("Sent Channel number");
      Serial.println(SCROLL_B);
 /*   while(gpio_get_level(keyPins[1]) == IDLE || pressed_button == false)
      {
        esp_err_t result = esp_now_send(macAddress[1], (uint8_t *) &SCROLL_B,sizeof(SCROLL_B));
        if (result != ESP_OK) 
          {
            Serial.println("Failed to send data");
          }
      delete m_transport;
      }
*/
    Serial.println("LAST LOOP");  
    int16_t *samples = reinterpret_cast<int16_t *>(malloc(sizeof(int16_t) * 128));
    while (true)
    {
      m_output_buffer->remove_samples(samples, 128);
      m_output->write(samples, 128);
      while (gpio_get_level(RECORD)==CLICKED)
      {
        Serial.println("STATE ON CLICKED");
        vTaskDelay(pdMS_TO_TICKS(100));
      }
      while (gpio_get_level(RECORD)==IDLE)
      {
      m_output_buffer->remove_samples(samples, 128);
      m_output->write(samples, 128);
        Serial.println("STATE ON IDLE");
        vTaskDelay(pdMS_TO_TICKS(100));

      }
    }
  }

void setupEspNowTransport(int init_channel, OutputBuffer* m_output_buffer)
{

    m_transport = new EspNowTransport(m_output_buffer,init_channel);
    m_transport->set_header(TRANSPORT_HEADER_SIZE,transport_header);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    m_transport->begin(1);
}
void waitForPress(gpio_num_t pin) 
{
  // 1) Wait until the pin reads “not pressed” (idle state)
  while (gpio_get_level(pin) != CLICKED)
    {
      vTaskDelay(pdMS_TO_TICKS(10));
    }
  // 2) Now that we’ve detected a press, wait until it’s released
  while (gpio_get_level(pin) == CLICKED) 
    {
      vTaskDelay(pdMS_TO_TICKS(10));
    }
}
void Channel_Screen(const char* title, const char* Channel)  /* draw title, number & arrows */
{
    u8g2.clearBuffer();                              // clear display buffer
    u8g2.setFont(u8g2_font_scrum_te);                
    int W  = u8g2.getDisplayWidth();   
    int H = u8g2.getDisplayHeight();              
    int fh = 12;                                      // font height for title
    u8g2.drawStr(50, 0, title);                     // title at (100, fh)
    u8g2.setFont(u8g2_font_NokiaSmallBold_tr);
    u8g2.drawHLine(0, fh + 2, W);                     // underline across screen
    u8g2.setFont(u8g2_font_fur35_tn);
    uint16_t numW = u8g2.getStrWidth(Channel);            // width of number
    uint16_t numH = u8g2.getMaxCharHeight();          // height of number
    int16_t cx = W/2 - numW/2;      
    int16_t cy = u8g2.getDisplayHeight()/2;     
    cy=cy-5;             // center X for number
    //int16_t cy = (u8g2.getDisplayHeight()/2) + numH/2; // center Y baseline
    u8g2.drawStr(cx, cy, Channel);                        // draw the number
    u8g2.setFont(u8g2_font_unifont_t_symbols);
    int16_t ax    = cx + numW + 2;                    // arrow X offset
    int16_t upY   = cy - numH + 40;   // just above the number
    int16_t downY = cy + numH -20;   // just below the number
    // int16_t upY   = cy - numH - 4;                    // arrow Y for up
    // int16_t downY = cy + 4;                           // arrow Y for down
    u8g2.drawUTF8(ax, upY,   "↑▲");              // up arrow
    u8g2.drawUTF8(ax, downY,      "↓▼");              // down arrow
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_NokiaSmallBold_tr );
    u8g2.drawStr(2, H - 10, "<<BACK");
    int selW = u8g2.getStrWidth("SELECT>>");
    u8g2.drawStr(256 - selW - 2, H - 10, "SELECT>>");
    u8g2.sendBuffer();                                // send buffer to display
}
void Menu_Screen(const char* title, const char* options[], uint8_t numoptions, uint8_t highlightIndex)
  {
    u8g2.clearBuffer();
    drawBatteryAndRSSI(65, -115," ");
    u8g2.setFont(u8g2_font_scrum_te );//u8g2_font_spleen6x12_mf 
                                      //u8g2_font_etl14thai_t 
                                      //u8g2_font_BitTypeWriter_tr
                                      //u8g2_font_NokiaSmallBold_tr 
  
    int W = u8g2.getDisplayWidth();
    int H = u8g2.getDisplayHeight();
    int fh = 12;  // Font height estimate

    u8g2.drawHLine(0, fh + 2, W);
    int optionY = 18;

    for(int i=0; i<4; i++)
      {
        if(i==highlightIndex)
          {
            u8g2.setFont(u8g2_font_NokiaSmallBold_tr );
            int textW = u8g2.getStrWidth(options[i]);
            // Draw white box
            u8g2.setDrawColor(1);
            u8g2.drawBox(100, optionY + (12 * i), textW + 12, fh);
            // Draw black text in box
            u8g2.setDrawColor(0);
            u8g2.drawStr(100, optionY+(12*i), options[i]);
            u8g2.setDrawColor(1);
          }
        else
          {
              u8g2.setDrawColor(1);
              u8g2.setFont(u8g2_font_NokiaSmallBold_tr );
              u8g2.drawStr(100, optionY+(12*i), options[i]);
          }
      }
    u8g2.setDrawColor(1);
    u8g2.drawStr(2, H - 10, "<<BACK");
    int selW = u8g2.getStrWidth("SELECT>>");
    u8g2.drawStr(256 - selW - 2, H - 10, "SELECT>>");
    u8g2.sendBuffer();
  }
void Navigate_Screen(const char* title)
  {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_scrum_te );//u8g2_font_spleen6x12_mf 
                                      //u8g2_font_etl14thai_t 
                                      //u8g2_font_BitTypeWriter_tr
                                      //u8g2_font_NokiaSmallBold_tr 
    int W = u8g2.getDisplayWidth();
    int H = u8g2.getDisplayHeight();
    int fh = 12;  // Font height estimate
    int f_pos=45;
    u8g2.drawStr(f_pos,0, title);
    u8g2.setFont(u8g2_font_NokiaSmallBold_tr );
    u8g2.drawHLine(0, fh + 2, W);
    int optionY = 18;
    u8g2.setDrawColor(1);
    u8g2.drawStr(2, H - 10, "<<BACK");
    int selW = u8g2.getStrWidth("START>>");
    u8g2.drawStr(256 - selW - 2, H - 10, "START>>");
    u8g2.sendBuffer();
  }
