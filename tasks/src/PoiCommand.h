#ifndef COMMAND_H
#define COMMAND_H

#include <Arduino.h>

enum PoiCommandType {
    NO_COMMAND,             // no command (default)
    BUTTON0_CLICK,          // button 0 was clicked
    BUTTON0_LONGCLICK,      // button 0 was clicked a long time
    BUTTON0_RELEASE,        // button 0 was pressed very long and released
    ACTION_CMD,             // a direct action like "stop animation"
    PROG_CMD,               // a program control command like "goto"
    PLAY_CMD                // a play command like "play frame 0 to 20" (either direct or in program)
};

class PoiCommand {
public:
    PoiCommand();
    PoiCommandType& getType();
    void setType(PoiCommandType type);
    bool isEmpty();
    void setArgs(uint8_t args[6]);
    uint8_t* getArgs();
private:
    PoiCommandType _type;   // the type of the command      
    uint8_t _args[6];       // arguments of the command (for wifi commands)
};

extern xQueueHandle commandQueue;

#endif