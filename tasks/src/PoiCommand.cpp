#include "PoiCommand.h"

PoiCommand::PoiCommand()
    :_type(NO_COMMAND){}

PoiCommandType& PoiCommand::getType(){
    return _type;
}

void PoiCommand::setType(PoiCommandType type){
    _type = type;
}    

bool PoiCommand::isEmpty(){
    return (_type == NO_COMMAND);
}    

void PoiCommand::setArgs(uint8_t[6] args){
    _args = args;
}