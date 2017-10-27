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

#include "Command.h"
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

static void buttonTask(void* arg)
{
  Command cmd;

    for(;;) {
      if(xQueueReceive(commandQueue, &cmd, portMAX_DELAY)) {

      if (cmd.getType() == BUTTON0_CLICK) {
        printf("--> CLICK\n");
      }
      else if (cmd.getType() == BUTTON0_LONGCLICK) {
        printf("--> LONG CLICK\n");
      }
      else if (cmd.getType() == BUTTON0_RELEASE) {
        printf("--> BUTTON RELEASED\n");
      }
      else {
        printf("--> UNKNOWN BUTTON EVENT: %d\n", static_cast<int>(cmd.getType()));
      }
    }
  }
}

void setup()
{
  buttonSetup();

  //create a queue to handle button commands from isr
  commandQueue = xQueueCreate(10, sizeof(Command));
  //start button task
  xTaskCreate(buttonTask, "buttonTask", 2048, NULL, 10, NULL);

}

void loop(){
  delay(100000);
}