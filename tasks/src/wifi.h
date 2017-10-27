#ifndef WIFI_H
#define WIFI_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "PoiCommand.h"

void wifiTask(void* arg);

#endif