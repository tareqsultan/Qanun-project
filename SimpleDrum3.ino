#include <Bounce.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <synth_simple_drum.h>
#include "Adafruit_MPR121.h"
#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;
// GUItool: begin automatically generated code
AudioSynthSimpleDrum     drum2;          //xy=399,244
AudioSynthSimpleDrum     drum3;          //xy=424,310
AudioSynthSimpleDrum     drum1;          //xy=431,197
AudioSynthSimpleDrum     drum4;          //xy=464,374
AudioMixer4              mixer1;         //xy=737,265
AudioOutputI2S           i2s1;           //xy=979,214
AudioConnection          patchCord1(drum2, 0, mixer1, 1);
AudioConnection          patchCord2(drum3, 0, mixer1, 2);
AudioConnection          patchCord3(drum1, 0, mixer1, 0);
AudioConnection          patchCord4(drum4, 0, mixer1, 3);
AudioConnection          patchCord5(mixer1, 0, i2s1, 0);
AudioConnection          patchCord6(mixer1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=930,518
// GUItool: end automatically generated code

static uint32_t next;

void setup() {
 
  Serial.begin(9600);

  while (!Serial) { // needed to keep leonardo/micro from starting too fast!
    delay(10);
  }
  
  Serial.println("Adafruit MPR121 Capacitive Touch sensor test"); 
  
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");
/////////////////////////////////////////////////////
  //Serial.begin(115200);

  // audio library init
  AudioMemory(15);

  //next = millis() + 1000;

  AudioNoInterrupts();

  drum1.frequency(60);
  drum1.length(1500);
  drum1.secondMix(0.0);
  drum1.pitchMod(0.55);
  
  drum2.frequency(80);
  drum2.length(300);
  drum2.secondMix(0.0);
  drum2.pitchMod(1.0);
  
  drum3.frequency(550);
  drum3.length(400);
  drum3.secondMix(1.0);
  drum3.pitchMod(0.5);

  drum4.frequency(1200);
  drum4.length(150);
  drum4.secondMix(0.0);
  drum4.pitchMod(0.0);

  
  
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);

  AudioInterrupts();

}

void loop() {  
  // ...  
  
  // Read the touch sensor values from the MPR121  
  uint16_t currtouched = cap.touched();  
  
  for (uint8_t i = 0; i < 12; i++) {  
  if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {  
  Serial.print(i); Serial.println(" touched");  
  switch (i) {  
    case 0:  
  drum1.noteOn();  
    break;  
   case 1:  
  drum2.noteOn();  
    break;  
    case 2:  
    drum3.noteOn();  
    break;  
    case 3:  
    drum4.noteOn();  
  break;  
    case 4:  
  drum1.noteOn();  
    break;  
    case 5:  
    drum2.noteOn();  
  break;  
  case 6:  
    drum3.noteOn();  
    break;  
    case 7:  
    drum4.noteOn();  
  break;  
    case 8:  
  drum1.noteOn();  
  break;  
   case 9:  
  drum2.noteOn();  
  break;  
    case 10:  
    drum3.noteOn();  
    break;  
    case 11:  
  drum4.noteOn();  
    break;  
  }  
  }  
  if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {  
  Serial.print(i); Serial.println(" released");  
  }  
  }  
  
  lasttouched = currtouched;  
  
  // ...  
}




