#include "button.h"

int _debounceTicks = 50;      // number of millisec that have to pass by before a click is assumed as safe.
int _longTicks = 600;         // number of millisec that have to pass by before a long button press is detected.
int _holdDownTicks = 2000;    // number of millisec after which we send a release


void IRAM_ATTR buttonIsrHandler(void* arg)
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
   xQueueSendFromISR(commandQueue, &click, NULL);
  }
}

void buttonSetup(){
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

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_0, buttonIsrHandler, (void*) GPIO_INPUT_IO_0);
}