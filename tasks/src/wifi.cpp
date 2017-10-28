#include "wifi.h"

void wifiTask(void* arg)
{
    uint8_t cmd_args[] = {10, 11, 12, 13, 14, 15};
    while (1) {

        PoiCommand cmd;
        cmd.setType(PLAY_CMD);
        cmd.setArgs(cmd_args);

        xQueueSend(commandQueue, &cmd, 0);

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}