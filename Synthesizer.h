#ifndef SYNTHESIZER_H  
#define SYNTHESIZER_H  
#include "Adafruit_MPR121.h"   


extern DaisyHardware pod;  
extern AdEnv env[16];  
extern Oscillator osc[16];  
extern Adafruit_MPR121 cap;  
extern uint16_t lasttouched;  
extern uint16_t currtouched;  
extern int s0;  
extern int s1;  
extern int s2;  
extern int Z_pin;  
extern int Z_pin1;  
  
void AudioCallback(float **in, float **out, size_t size);  
void synth_setup();  
void synth_loop();  
  
#endif  // SYNTHESIZER_H
