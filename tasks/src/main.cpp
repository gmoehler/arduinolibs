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
          printf("--> PLAY %d %d %d %d %d %d \n", a[0], a[1], a[3], a[4], a[5]);
          break;

        default:
          printf("--> UNKNOWN BUTTON EVENT: %d\n", static_cast<int>(cmd.getType()));
      }
    }
  }
}

void setup()
{
  buttonSetup();

  //create a queue to handle button commands from isr
  commandQueue = xQueueCreate(10, sizeof(PoiCommand));
  //start button task
  xTaskCreate(adminTask, "adminTask", 2048, NULL, 10, NULL);
  xTaskCreate(wifiTask, "wifiTask", 2048, NULL, 10, NULL);

}

void loop(){
  delay(100000);
}