#include "mock_Arduino.h"

uint32_t _testCurrentTime=0L;

unsigned long millis() {
  printf("millis: %d\n",_testCurrentTime );
  return _testCurrentTime;
}

void mock_increaseTime(uint32_t ms){
    _testCurrentTime+= ms;
}

void delay(uint32_t ms) {
  printf("delay:  %d \n", ms);
  mock_increaseTime(ms);
}

void pinMode(int pin, int mode){
  printf("Set pinMode for %d to %d", pin, mode);
}

int digitalRead(int pin){
  return LOW;
}

uint8_t constrain(uint8_t value, uint8_t lowerBound, uint8_t upperBound){
  if (value > upperBound){
    return upperBound;
  }
  if (value < lowerBound){
    return lowerBound;
  }
  return value;
}

String::String(const char* cstr)
  :_buffer(cstr){};

const char* String::c_str() {
  return _buffer.data();
}
bool String::equals(String str){
  return _buffer == str.c_str();
}

HardwareSerial Serial;

size_t HardwareSerial::print(const char str[]) {
  return printf("%s", str);
}
size_t HardwareSerial::println(const char str[]) {
  return printf("%s\n", str);
}
size_t HardwareSerial::println() {
  return printf("\n");
}



