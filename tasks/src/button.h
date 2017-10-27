#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "PoiCommand.h"

#define GPIO_INPUT_IO_0       GPIO_NUM_0
#define GPIO_INPUT_PIN_SEL    (1<<GPIO_INPUT_IO_0)
#define ESP_INTR_FLAG_DEFAULT 0

void IRAM_ATTR buttonIsrHandler(void* arg);
void buttonSetup();

#endif