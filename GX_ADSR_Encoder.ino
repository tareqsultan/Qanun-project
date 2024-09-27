#include <Encoder.h>  
#include <GyverOLED.h>  
  
#define ENCODER_PIN_A 38  
#define ENCODER_PIN_B 39  
#define ENCODER_PUSHBUTTON_PIN 40  
  
GyverOLED<SSH1106_128x64> display;  
  
Encoder myEnc(ENCODER_PIN_B, ENCODER_PIN_A);  
  
int attackTime = 20;  
int decayTime = 30;  
int sustainTime = 40;  
int releaseTime = 50;  
  
int currentPoint = 0; // 0: attack, 1: decay, 2: sustain, 3: release  
  
void setup() {  
  pinMode(ENCODER_PUSHBUTTON_PIN, INPUT);  
  display.init();  
}  
  
int B = 20;  
int C = 30;  
int D = 30;  
int E = 50;  
  
void loop() {  
  long encoderValue = myEnc.read();  
  int pushbuttonState = digitalRead(ENCODER_PUSHBUTTON_PIN);  
  
  if (pushbuttonState == HIGH) {  
 // increment or decrement current point  
  currentPoint = (currentPoint + 1) % 4;  
  }  
  
  switch (currentPoint) {  
  case 0:  
  B = map(encoderValue, 0, 1023, 10, 60);  
  break;  
  case 1:  
  C = map(encoderValue, 0, 1023, 10, 60);  
  break;  
  case 2:  
  D = map(encoderValue, 0, 1023, 10, 50);  
  break;  
  case 3:  
  E = map(encoderValue, 0, 1023, 10, 60);  
  break;  
  }  
  
  // Store the values in separate variables  
  int attackValue = B;  
  int decayValue = C;  
  int sustainValue = D;  
  int releaseValue = E;  
  
  displayADSR(attackValue, decayValue, sustainValue, releaseValue);  
  delay(10);  
}  
  
void displayADSR(int attackValue, int decayValue, int sustainValue, int releaseValue) {  
  display.clear();  
  display.setCursor(0, 0);  
  switch (currentPoint) {  
  case 0:  
  display.print("ENV:Attack: ");  
  display.print(attackValue);  
  break;  
  case 1:  
  display.print("ENV:Decay: ");  
  display.print(decayValue);  
  break;  
  case 2:  
  display.print("ENV:Sustain: ");  
  display.print(sustainValue);  
  break;  
  case 3:  
  display.print("ENV:Release: ");  
  display.print(releaseValue);  
  break;  
  }  
  display.rect(0, 10, 120, 60, OLED_STROKE);  
  // Draw ADSR envelope  
  int attackX = map(attackValue, 10, 60, 0, 40);  
  int attackY = 10;  
  int decayX = map(decayValue, 10, 60, 40, 80); // change end point X for decay  
  int decayY = map(sustainValue, 10, 60, 10, 50); // connect to sustainY start point  
  int sustainX = decayX; // change X point of sustain  
  int sustainY = decayY; // sustainY start and end are the same  
  int releaseStartX = map(releaseValue, 10, 60, sustainX, 120); // releaseX start is connected to sustainX end  
  int releaseStartY = sustainY; // releaseY start is connected to sustainY  
  int releaseEndX = 120; // fixed end point X for release  
  int releaseEndY = 60; // fixed end point Y for release  
  
  display.line(0, 60, attackX, attackY);  
  display.line(attackX, attackY, decayX, decayY);  
  display.line(decayX, decayY, sustainX, sustainY);  
  display.line(sustainX, sustainY, releaseStartX, releaseStartY); // connect sustainX end to releaseX start  
  display.line(releaseStartX, releaseStartY, releaseEndX, releaseEndY);  
  display.update();  
}








