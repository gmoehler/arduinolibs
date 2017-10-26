/* GPIO Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

/**
 * Brief:
 * This test code shows how to configure gpio and how to use gpio interrupt.
 *
 * GPIO status:
 * GPIO0:  input, pulled up, interrupt from rising edge and falling edge
 *
 * Test:
 * Press GPIO0, this triggers interrupt
 *
 */

#define GPIO_INPUT_IO_0     0
#define GPIO_INPUT_PIN_SEL  (1<<GPIO_INPUT_IO_0)
#define ESP_INTR_FLAG_DEFAULT 0

int _debounceTicks = 50;      // number of millisec that have to pass by before a click is assumed as safe.
int _longTicks = 600;        // number of millisec that have to pass by before a long button press is detected.


typedef struct {
  uint32_t time;  // the PCNT unit that originated an interrupt
  int level; // information on the event type that caused the interrupt
} button_evt_t;

static xQueueHandle gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    button_evt_t evt;
    uint32_t gpio_num = (uint32_t) arg;
    evt.time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    evt.level = gpio_get_level(gpio_num);
    xQueueSendFromISR(gpio_evt_queue, &evt, NULL);
}

static void gpio_task_example(void* arg)
{
  static button_evt_t prev_evt;
  static button_evt_t prev_evt_pressed;
  button_evt_t evt;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &evt, portMAX_DELAY)) {
          printf("Button: %d ms, val: %d\n", evt.time, evt.level);
          printf("Button-prev: %d ms, val: %d\n", prev_evt_pressed.time, prev_evt_pressed.level);

          if (evt.level == 0){
              prev_evt_pressed = evt;
          }
          if (evt.level == 1){
            int presstime = evt.time - prev_evt_pressed.time;
            printf("Press time: %d ms\n", presstime );

            if (presstime < _debounceTicks) {
              printf("--> NO CLICK\n");
            }
            else if (presstime < _longTicks) {
              printf("--> CLICK\n");
            }
            else if (presstime >= _longTicks) {
              printf("--> LONG CLICK\n");
            }
         }
        }
    }
}

void app_main()
{
    gpio_config_t io_conf;
    //interrupt of rising edge
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    //bit mask of the pins, use GPIO0 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //change gpio intrrupt type for one pin
    gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_ANYEDGE);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(button_evt_t));
    //start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);

}

