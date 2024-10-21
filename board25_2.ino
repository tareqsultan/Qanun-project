#include <GyverOLED.h>  
#include <DaisyDuino.h>  
  
GyverOLED<SSH1106_128x64> display;  
DaisyHardware hw;  
extern Oscillator osc[16];  
int encoderValue = 0;  
int option = 0;  
int submenuOption = 0;  
int currentPoint = 0;    

void setup() {  
  
  hw = DAISY.init(DAISY_POD, AUDIO_SR_48K);  
  display.init();  
  synth_setup();  
}  
/*  
void loop() {  
  synth_loop();  
  hw.DebounceControls();  
  
  int increment = hw.encoder.Increment();  
  if (increment != 0) {  
  encoderValue += increment;  
  encoderValue = (encoderValue % 2 + 2) % 2;  
  
  display.clear();  
  display.setCursor(0, 0);  
  display.print("MENU");  
  display.setCursor(0, 1);  
  if (encoderValue == 1) {  
  display.invertText(true);  
  display.print("Waveform");  
  display.invertText(false);  
  display.setCursor(0, 2);  
  display.print("Envelope");  
  } else {  
  display.print("Waveform");  
  display.setCursor(0, 2);  
  display.invertText(true);  
  display.print("Envelope");  
  display.invertText(false);  
  }  
  display.update();  
  }  
  
  if (hw.encoder.RisingEdge()) {  
  if (encoderValue == 1) {  
  Waveform(); // Call the Waveform function from Waveform tab  
  } else if (encoderValue == 0) {  
  Envelope(); // Call the Envelope function from Envelope tab  
  }  
  }  
  
  UpdateKnobs();  
}  
*/
void loop() {  
  synth_loop();  
  hw.DebounceControls();  
  
  int increment = hw.encoder.Increment();  
  if (increment != 0) {  
   encoderValue += increment;  
   encoderValue = (encoderValue % 3 + 3) % 3;  
  
   display.clear();  
   display.setCursor(0, 0);  
   display.print("MENU");  
   display.setCursor(0, 1);  
   if (encoderValue == 0) {  
    display.invertText(true);  
    display.print("Waveform");  
    display.invertText(false);  
    display.setCursor(0, 2);  
    display.print("Envelope");  
    display.setCursor(0, 3);  
    display.print("Effect");  
   } else if (encoderValue == 1) {  
    display.print("Waveform");  
    display.setCursor(0, 2);  
    display.invertText(true);  
    display.print("Envelope");  
    display.invertText(false);  
    display.setCursor(0, 3);  
    display.print("Effect");  
   } else if (encoderValue == 2) {  
    display.print("Waveform");  
    display.setCursor(0, 2);  
    display.print("Envelope");  
    display.setCursor(0, 3);  
    display.invertText(true);  
    display.print("Effect");  
    display.invertText(false);  
   }  
   display.update();  
  }  
  
  if (hw.encoder.RisingEdge()) {  
   if (encoderValue == 0) {  
    Waveform(); // Call the Waveform function from Waveform tab  
   } else if (encoderValue == 1) {  
    Envelope(); // Call the Envelope function from Envelope tab  
   } else if (encoderValue == 2) {  
    Effect(); // Call the Effect function from Effect tab  
   }  
  }  
}