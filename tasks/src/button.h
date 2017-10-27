#ifndef BUTTON_H
#define BUTTON_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define GPIO_INPUT_IO_0     GPIO_NUM_0
#define GPIO_INPUT_PIN_SEL  (1<<GPIO_INPUT_IO_0)
#define ESP_INTR_FLAG_DEFAULT 0

typedef enum {
  BUTTON_NOCLICK,
  BUTTON_CLICK,
  BUTTON_LONGCLICK,
  // BUTTON_HOLDDOWN,
  BUTTON_RELEASE
} ButtonClickType;

static xQueueHandle gpio_evt_queue = NULL;


static void IRAM_ATTR button_isr_handler(void* arg);

void button_setup();
void button_setup2();

#endif