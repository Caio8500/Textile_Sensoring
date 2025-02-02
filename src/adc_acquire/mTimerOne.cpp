#include <Arduino.h>



// https://www.instructables.com/Arduino-Timer-Interrupts/

void startTimerOne(){
  // enable timer 1 compare interrupt
  TIMSK1 |= (1 << OCIE1A);
}
void stopTimerOne(){
  // disable timer 1 compare interrupt
  TIMSK1 &= ~(1 << OCIE1A); 
}

void setupTimerOne(){
  /* This function sets up the arduino's timer one
  :param interruptFreq: interruption frequency
  :param prescaler: timer's prescaler
  :return: void
  */
  // setting up interrupt freq to 32 Hz
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; //initialize counter value to 0
  OCR1A = 3905;// = (16*10^6) / (prescaler*samplingFreq) - 1 (must be <65536) -> = [(clk) / (presclaer*samplingFreq)] - 1
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // 64 prescaler 
  TCCR1B |= (1 << CS11) | (1 << CS10); // shift left b01 CS11 times -> b10 
}