/* GPIO Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "Arduino.h"

#include "button.h"

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

static void button_task_example(void* arg)
{
  ButtonClickType newclick;

    for(;;) {
      if(xQueueReceive(gpio_evt_queue, &newclick, portMAX_DELAY)) {

      if (newclick == BUTTON_CLICK) {
        printf("--> CLICK\n");
      }
      else if (newclick == BUTTON_LONGCLICK) {
        printf("--> LONG CLICK\n");
      }
      else if (newclick == BUTTON_RELEASE) {
        printf("--> BUTTON RELEASED\n");
      }
      else {
        printf("--> UNKNOWN BUTTON EVENT: %d\n", newclick);
      }
    }
  }
}

void setup()
{
  button_setup();

  //create a queue to handle gpio event from isr
  gpio_evt_queue = xQueueCreate(10, sizeof(ButtonClickType));
  //start gpio task
  xTaskCreate(button_task_example, "button_task_example", 2048, NULL, 10, NULL);

  button_setup2();
}

void loop(){
  delay(100000);
}