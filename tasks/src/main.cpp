/* GPIO Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "Arduino.h"

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

#define GPIO_INPUT_IO_0     GPIO_NUM_0
#define GPIO_INPUT_PIN_SEL  (1<<GPIO_INPUT_IO_0)
#define ESP_INTR_FLAG_DEFAULT 0

int _debounceTicks = 50;      // number of millisec that have to pass by before a click is assumed as safe.
int _longTicks = 600;         // number of millisec that have to pass by before a long button press is detected.
int _holdDownTicks = 2000;    // number of millisec after which we send a release

typedef enum {
  BUTTON_NOCLICK,
  BUTTON_CLICK,
  BUTTON_LONGCLICK,
  // BUTTON_HOLDDOWN,
  BUTTON_RELEASE
} ButtonClickType;

static xQueueHandle gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
  static uint32_t lastPressedTime = 0;

  gpio_num_t gpioNum = (gpio_num_t) reinterpret_cast<int>(arg);
  int level = gpio_get_level(gpioNum);
  uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
  int pressDuration = now - lastPressedTime;

  ButtonClickType click = BUTTON_NOCLICK;
 
  // determine type of click when button is released
  if (level == 1 && lastPressedTime > 0){

    if (pressDuration < _debounceTicks) {
      click = BUTTON_NOCLICK;
    }
    else if (pressDuration < _longTicks) {
      click = BUTTON_CLICK;
    }
    else if (pressDuration >= _holdDownTicks) {
      click = BUTTON_RELEASE;
    }
    else if (pressDuration >= _longTicks) {
      click = BUTTON_LONGCLICK;
    }
  } 

  // remember time when button is pressed down
  else if (level == 0){
    lastPressedTime = now;
  }

  if (click != BUTTON_NOCLICK){
    // clear press memory
   lastPressedTime = 0;
   // send out event
   xQueueSendFromISR(gpio_evt_queue, &click, NULL);
  }
}

static void gpio_task_example(void* arg)
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
    gpio_config_t io_conf;
    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    //bit mask of the pins, use GPIO0 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    //change gpio intrrupt type for one pin
    gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_ANYEDGE);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(ButtonClickType));
    //start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);

}

void loop(){
  delay(100000);
}