#ifndef COMMAND_H
#define COMMAND_H

#include <Arduino.h>

enum CommandType {
    NO_COMMAND,             // no command (default)
    BUTTON0_CLICK,          // button 0 was clicked
    BUTTON0_LONGCLICK,      // button 0 was clicked a long time
    BUTTON0_RELEASE,        // button 0 was pressed very long and released
    ACTION_CMD,             // a direct action like "stop animation"
    PROG_CMD,               // a program control command like "goto"
    PLAY_CMD                // a play command like "play frame 0 to 20" (either direct or in program)
};

class Command {
public:
    Command();
    CommandType& getType();
    void setType(CommandType type);
    bool isEmpty();
private:
    CommandType _type;       // the type of the command      
    uint32_t _args[6];       // arguments of the command (for wifi commands)
};

#endif