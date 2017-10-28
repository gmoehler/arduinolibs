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

void PoiCommand::setArgs(uint8_t args[6]){
    for (int i=0; i<6; i++){
        _args[i] = args[i];
    }
}

uint8_t* PoiCommand::getArgs(){
    return _args;
}