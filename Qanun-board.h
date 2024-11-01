//Qanun-board.h
#ifndef DEVICES_H  
#define DEVICES_H  
  
// DaisyDuino board  
#include <DaisyDuino.h>  
DaisyHardware pod;  
  
// OLED display  
#include <GyverOLED.h>  
GyverOLED<SSH1106_128x64> display;  
  
// MPR121 capacitive touch sensor  
#include <Adafruit_MPR121.h>  
Adafruit_MPR121 cap = Adafruit_MPR121();  
Adafruit_MPR121 cap2 = Adafruit_MPR121();  
  
// MUX (multiplexer)  
int s0 = 9;  
int s1 = 8;  
int s2 = 7;  
int Z_pin = 7;  
int Z_pin1 = A1;  
  
// MUX read functions  
int readMux1(int channel) {  
  int controlPin[] = {s0, s1, s2};  
  int muxChannel[8][3] = {  
   {0, 0, 0}, // channel 0  
   {1, 0, 0}, // channel 1  
   {0, 1, 0}, // channel 2  
   {1, 1, 0}, // channel 3  
   {0, 0, 1}, // channel 4  
   {1, 0, 1}, // channel 5  
   {0, 1, 1}, // channel 6  
   {1, 1, 1}, // channel 7  
  };  
  
  for (int i = 0; i < 3; i++) {  
   digitalWrite(controlPin[i], muxChannel[channel][i]);  
  }  
  return analogRead(Z_pin);  
}  
  
int readMux2(int channel) {  
  int controlPin[] = {s0, s1, s2};  
  int muxChannel[8][3] = {  
   {0, 0, 0}, // channel 0  
   {1, 0, 0}, // channel 1  
   {0, 1, 0}, // channel 2  
   {1, 1, 0}, // channel 3  
   {0, 0, 1}, // channel 4  
   {1, 0, 1}, // channel 5  
   {0, 1, 1}, // channel 6  
   {1, 1, 1}, // channel 7  
  };  
  
  for (int i = 0; i < 3; i++) {  
   digitalWrite(controlPin[i], muxChannel[channel][i]);  
  }  
  return analogRead(Z_pin1);  
}  
  
#endif  // DEVICES_H