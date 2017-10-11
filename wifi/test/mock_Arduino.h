#ifndef mock_arduino_h
#define mock_arduino_h

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// functions mocking Arduino.h to be used

#define LOW 0
#define HIGH 1

unsigned long millis();
void delay(unsigned long ms);

void pinMode(int, int);
int digitalRead(int pin);

uint8_t constrain(uint8_t value, uint8_t lowerBound, uint8_t upperBound);

// functions controling the behaviour of the mocks

void mock_increaseTime(uint32_t ms);

class HardwareSerial
{
public:
  size_t println(const char[]);
  size_t println();
  size_t print(const char[]);
  size_t print();
};
extern HardwareSerial Serial;

class String {
public:
  String(const char* cstr="") {strcpy(_buffer, cstr);};
  char* c_str() {return _buffer;}
private:
  char*_buffer;
};

#endif
