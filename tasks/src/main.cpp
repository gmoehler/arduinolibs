/* GPIO Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "PoiCommand.h"
#include "button.h"
#include "wifi.h"
#include "ws2812.h"

/**
 * Brief:
 * Detect button clicks and longclicks and send it to a queue.
 *
 * GPIO status:
 * GPIO0:  input, pulled up, interrupt from rising edge and falling edge
 *
 * Test:
 * Press GPIO0, this triggers interrupt
 *
 */

xQueueHandle commandQueue = NULL;
volatile SemaphoreHandle_t frameTimerSemaphore;
hw_timer_t * frameTimer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

const int DATA_PIN = 23; // was 18 Avoid using any of the strapping pins on the ESP32
const uint16_t NUM_PIXELS = 30;
rgbVal *pixels;
uint8_t MAX_COLOR_VAL = 32; // Limits brightness

// reads from command queue
static void adminTask(void* arg)
{
  PoiCommand cmd;

    for(;;) {
      if(xQueueReceive(commandQueue, &cmd, portMAX_DELAY)) {
      uint8_t* a = cmd.getArgs();  

      switch (cmd.getType()){   
        case BUTTON0_CLICK :
          printf("--> CLICK\n");
          break;
        
        case BUTTON0_LONGCLICK :
          printf("--> LONG CLICK\n");
          break;
          
        case BUTTON0_RELEASE:
          printf("--> BUTTON RELEASED\n");
          break;
        
        case PLAY_CMD:
          printf("--> PLAY %d %d %d %d %d %d \n", a[0], a[1], a[2], a[3], a[4], a[5]);
          break;

        default:
          printf("--> UNKNOWN BUTTON EVENT: %d\n", static_cast<int>(cmd.getType()));
      }
    }
  }
}

static void playTask(void* arg)
{
  for(;;) {
    if (xSemaphoreTake(frameTimerSemaphore, portMAX_DELAY)) {
      printf("Playing...\n");

      for(uint16_t i=0; i<NUM_PIXELS; i++) {
        pixels[i] = makeRGBVal(MAX_COLOR_VAL,0,0);
      }
      //ws2812_setColors(NUM_PIXELS, pixels);
    }
  }
}

void IRAM_ATTR onFrameTimer(){
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  // nothing critical for now
  portEXIT_CRITICAL_ISR(&timerMux);
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(frameTimerSemaphore, NULL);
  // It is safe to use digitalRead/Write here if you want to toggle an output
}


void setup()
{
  buttonSetup();

  //create a queue to handle button commands from isr
  commandQueue = xQueueCreate(10, sizeof(PoiCommand));
  //start button task
  xTaskCreate(adminTask, "adminTask", 2048, NULL, 10, NULL);
  xTaskCreate(wifiTask, "wifiTask", 2048, NULL, 10, NULL);


  if(ws2812_init(DATA_PIN, LED_WS2812B)){
    Serial.println("LED Pixel init error.");
  }
  pixels = (rgbVal*)malloc(sizeof(rgbVal) * NUM_PIXELS);
  

  // create and start frame timer and task
  frameTimerSemaphore = xSemaphoreCreateBinary();
    frameTimer = timerBegin(3, 80, true); // start timer 0 counting up
  timerAttachInterrupt(frameTimer, &onFrameTimer, true);
  timerAlarmWrite(frameTimer, 1000000, true); // time in millisecs
  timerAlarmEnable(frameTimer);
  xTaskCreate(playTask, "playTask", 2048, NULL, 10, NULL);
  
}

void loop(){
  delay(100000);
}