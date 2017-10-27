#include "Command.h"

Command::Command()
    :_type(NO_COMMAND){}

CommandType& Command::getType(){
    return _type;
}

void Command::setType(CommandType type){
    _type = type;
}    

bool Command::isEmpty(){
    return (_type == NO_COMMAND);
}    