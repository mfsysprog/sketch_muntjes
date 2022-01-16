  
#include <TJpg_Decoder.h>
#include <SPIFFS.h>
#include <Arduino_GFX_Library.h>
#include "FreeSerifBoldItalic12pt7b.h"
#include "driver/pcnt.h"
     
#define TFT_MOSI 19
#define TFT_SCLK 18
#define TFT_CS 5
#define TFT_DC 16
#define TFT_RST 23
#define TFT_BL 4

#define COUNT_PIN       15  // the gpio I output pulses on which I want to count

Arduino_ESP32SPI *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO);
Arduino_ST7789 *display = new Arduino_ST7789(bus, 2 /* RST */, 1 /* rotation */, true /* IPS */, 135 /* width */, 240 /* height */, 53 /* col offset 1 */, 40 /* row offset 1 */, 52 /* col offset 2 */, 40 /* row offset 2 */);
//Arduino_ST7789 *display = new Arduino_ST7789(bus, TFT_RST);

//volatile xQueueHandle pcnt_evt_queue;   // A queue to handle pulse counter events

/* A sample structure to pass events from the PCNT
 * interrupt handler to the main program.
 */
//typedef struct {
//    int unit;  // the PCNT unit that originated an interrupt
//    uint32_t status; // information on the event type that caused the interrupt
//} pcnt_evt_t;

pcnt_config_t pcnt_config = {
        .pulse_gpio_num = COUNT_PIN,            // set gpio for pulse input gpio
        .ctrl_gpio_num = -1,            // no gpio for control
        .lctrl_mode = PCNT_MODE_KEEP,   // when control signal is low, keep the primary counter mode
        .hctrl_mode = PCNT_MODE_KEEP,   // when control signal is high, keep the primary counter mode
        .pos_mode = PCNT_COUNT_INC,     // increment the counter on positive edge
        .neg_mode = PCNT_COUNT_DIS,     // do nothing on falling edge
        .counter_h_lim = 5000,
        .counter_l_lim = 0,
        .unit = PCNT_UNIT_0,               /*!< PCNT unit number */
        .channel = PCNT_CHANNEL_0
    };
    
//pcnt_isr_handle_t user_isr_handle = NULL; //user's ISR service handle

const char* filePath0 = "/euro_5.jpg";
const char* filePath1 = "/euro_10.jpg";
const char* filePath2 = "/euro_20.jpg";
const char* filePath3 = "/euro_50.jpg";
const char* filePath4 = "/euro_100.jpg";
const char* filePath5 = "/euro_200.jpg";
int nBlocks = 0;
int test_count = 0;
int16_t PulseCounter =     0; 
int16_t PulseCounter_second =     0; 
int16_t PulseCounter_prev =     0; 
int16_t limit = 4;
uint16_t eurocent = 0;
bool coin = true;
//int16_t pcnt_count = 0;
//pcnt_evt_t evt;
//portBASE_TYPE res;
//int pcnt_unit = PCNT_UNIT_0;
//int16_t count = 0;

//static void pcnt_intr_handler(void *arg)
//{
//    int pcnt_unit = (int)arg;
//    pcnt_evt_t evt;
//    evt.unit = pcnt_unit;
//    /* Save the PCNT event type that caused an interrupt
//       to pass it to the main program */
//    pcnt_get_event_status((pcnt_unit_t)pcnt_unit, &evt.status);
//    xQueueSendFromISR(pcnt_evt_queue, &evt, NULL);
////    int pcnt_unit = (int)arg;
////    pcnt_count++;
//    pcnt_intr_disable(PCNT_UNIT_0);
////    Serial.println("Interrupt called!\n");
////    esp_intr_free(user_isr_handle);
////    user_isr_handle = NULL;
//}
  
void setup() {
  Serial.begin(115200);
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS init failed");
    return;
  }
  Serial.println("SPIFFS init finished");
  display->begin(80000000); // 80 MHz
    
  uint16_t imgWidth, imgHeight;
  int result = TJpgDec.getFsJpgSize(&imgWidth, &imgHeight, filePath0);
  Serial.printf("Result: %i\n", result);
 
  Serial.println("\nImage dimensions:");
  Serial.printf("width = %d, height = %d\n", imgWidth, imgHeight);

  ledcSetup(0 /* LEDChannel */, 5000 /* freq */, 8 /* resolution */);
  ledcAttachPin(TFT_BL, 0 /* LEDChannel */);
  ledcWrite(0 /* LEDChannel */, 80); /* 0-255 */
//  pinMode(TFT_BL, OUTPUT);
//  digitalWrite(TFT_BL, HIGH);
 
  TJpgDec.setCallback(onDecodeBlock);

  pinMode(COUNT_PIN,INPUT);

  pcnt_unit_config(&pcnt_config);        //init unit
//  pcnt_unit_config(&pcnt_config_2);        //init unit
//  pcnt_counter_clear(PCNT_UNIT_1);
//  pcnt_set_filter_value(PCNT_UNIT_0, test_count);
//  pcnt_filter_enable(PCNT_UNIT_0);
  /* Register ISR handler and enable interrupts for PCNT unit */
//  pcnt_set_event_value(PCNT_UNIT_0, PCNT_EVT_THRES_0, limit);
////  pcnt_event_enable(PCNT_UNIT_0, PCNT_EVT_THRES_0); 
//  pcnt_event_enable(PCNT_UNIT_0, PCNT_EVT_H_LIM); 
  
  pcnt_counter_pause(PCNT_UNIT_0);
  pcnt_counter_clear(PCNT_UNIT_0);
  
  //pcnt_isr_service_install(0);
//  pcnt_isr_register(pcnt_intr_handler, NULL, 0, &user_isr_handle);
//  pcnt_intr_enable(PCNT_UNIT_0);
//  pcnt_intr_enable(PCNT_UNIT_1);

  /* Everything is set up, now go to counting */
  pcnt_counter_resume(PCNT_UNIT_0);
//  pcnt_counter_resume(PCNT_UNIT_1);
//  pcnt_evt_queue = xQueueCreate(10, sizeof(pcnt_evt_t));
}

void loop() {
  //loopje om tijdens tellen muntpulsen te wachten tot alle pulsen getest zijn.
  do
  {
    pcnt_get_counter_value(PCNT_UNIT_0, &PulseCounter);
    delay(120);
    pcnt_get_counter_value(PCNT_UNIT_0, &PulseCounter_second);
  } while (PulseCounter != PulseCounter_second);
  
  switch (PulseCounter - PulseCounter_prev)
  {
  case 0:
    if (coin)
    {
      display->fillScreen(WHITE);
      display->setFont(&FreeSerifBoldItalic12pt7b);
      display->setCursor(10, 50);
      display->setTextColor(RED);
      display->println("Centjes gespaard:");
      display->setCursor(10, 80);
      display->setTextColor(DARKGREEN);
      display->printf("%d Euro en %d Cent", (uint16_t)floor(((float)eurocent/100)), (uint16_t)( eurocent % 100 ));
      coin = false;
    }
//    display->setCursor(10, 90);
//    display->setTextColor(BLACK);
//    display->println("Centjes voor Mama");
    break;
  case 1:
    TJpgDec.drawFsJpg(0, 0, filePath0);
    PulseCounter_prev = PulseCounter;
    eurocent += 5;
    coin = true;
    break;
  case 2:
    TJpgDec.drawFsJpg(0, 0, filePath1);
    PulseCounter_prev = PulseCounter;
    eurocent += 10;
    coin = true;
    break;
  case 3:
    TJpgDec.drawFsJpg(0, 0, filePath2);
    PulseCounter_prev = PulseCounter;
    eurocent += 20;
    coin = true;
    break;
  case 4:
    TJpgDec.drawFsJpg(0, 0, filePath3);
    PulseCounter_prev = PulseCounter;
    eurocent += 50;
    coin = true;
    break;
  case 5:
    TJpgDec.drawFsJpg(0, 0, filePath4);
    PulseCounter_prev = PulseCounter;
    eurocent += 100;
    coin = true;
    break;
  case 6:
    TJpgDec.drawFsJpg(0, 0, filePath5);
    PulseCounter_prev = PulseCounter;
    eurocent += 200;
    coin = true;
    break;
  default:
    display->fillScreen(RED);
    display->setFont(&FreeSerifBoldItalic12pt7b);
    display->setCursor(10, 50);
    display->setTextColor(WHITE);
    display->println("Fout bij tellen...");
  }
  delay(500);    
  Serial.printf("Counter at %d, previous at %d for %d eurocent\n", PulseCounter, PulseCounter_prev, eurocent);

//  res = xQueueReceive(pcnt_evt_queue, &evt, 1000 / portTICK_PERIOD_MS);
//        if (res == pdTRUE) {
//            pcnt_get_counter_value((pcnt_unit_t)pcnt_unit, &count);
//            Serial.printf("Event PCNT unit[%d]; cnt: %d\n", evt.unit, count);
//            if (evt.status & PCNT_EVT_THRES_1) {
//                Serial.println("THRES1 EVT\n");
//            }
//            if (evt.status & PCNT_EVT_THRES_0) {
//                Serial.println("THRES0 EVT\n");
//                pcnt_counter_pause(PCNT_UNIT_0);
//                pcnt_counter_clear(PCNT_UNIT_0);
//                pcnt_counter_resume(PCNT_UNIT_0);
//                pcnt_set_event_value(PCNT_UNIT_0, PCNT_EVT_THRES_0, limit);
//                pcnt_event_disable(PCNT_UNIT_0, PCNT_EVT_THRES_0); 
//                esp_intr_free(user_isr_handle);
//                user_isr_handle = NULL;
//                pcnt_isr_register(pcnt_intr_handler, NULL, 0, &user_isr_handle);
//                pcnt_intr_enable(PCNT_UNIT_0);
//            }
//            if (evt.status & PCNT_EVT_L_LIM) {
//                Serial.println("L_LIM EVT\n");
//            }
//            if (evt.status & PCNT_EVT_H_LIM) {
//                Serial.println("H_LIM EVT\n");
//                pcnt_counter_pause(PCNT_UNIT_0);
//                pcnt_counter_clear(PCNT_UNIT_0);
//                pcnt_counter_resume(PCNT_UNIT_0);
//                pcnt_intr_enable(PCNT_UNIT_0);
//            }
//            if (evt.status & PCNT_EVT_ZERO) {
//                Serial.println("ZERO EVT\n");
//            }
//        } else {
//            pcnt_get_counter_value((pcnt_unit_t)pcnt_unit, &count);
//            Serial.printf("Current counter value :%d\n", count);
//        }
}

bool onDecodeBlock(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
//  nBlocks = nBlocks + 1;
//   
//  Serial.println("\n----------");
//  Serial.printf("Nº of blocks %d\n", nBlocks);
//  Serial.printf("x = %d, y = %d\n", x, y);
//  Serial.printf("width = %d, height = %d\n", w, h);
//  Serial.flush();
  
  display->draw16bitRGBBitmap(x, y, bitmap, w, h);
   
  return 1;
}
