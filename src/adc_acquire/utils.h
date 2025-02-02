#include <Arduino.h>
#include <OneWire.h>

#ifndef utils_h
#define utils_h


// MACROS 
// this macros will select which print statements will be executed according to the chosen mode (DEBUG or RELEASE)
#define DEBUG 0 // 1 - Debug; 0 - Release
#if DEBUG == 1
  #define LOG_DEBUG(x) {Serial.print(x);}
  #define LOG_DEBUG_LN(x) {Serial.println(x);}
  #define LOG_DEBUG_LN_HEX(x){Serial.print("0X"); \
                              Serial.println(x, HEX);}     
#else
  #define LOG_DEBUG(x) {}// if it's release mode, this funcion will do nothing
  #define LOG_DEBUG_LN(x) {}// if it's release mode, this funcion will do nothing
  #define LOG_DEBUG_LN_HEX(x) {}
#endif
namespace mUtils{
void multiplex(uint8_t currentMuxChannel, char currentMux, const int csaPin, const int csbPin, const int wrPin, const int enPin, const uint8_t *addrMux);
float getTemperatureDS18b20(OneWire d, const int DS18B20_ID);

}

#endif