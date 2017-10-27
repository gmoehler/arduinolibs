#include "button.h"

static void IRAM_ATTR button_isr_handler(void* arg)
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