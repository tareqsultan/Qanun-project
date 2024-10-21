#include <GyverOLED.h>  
#include <DaisyDuino.h>  
  
//GyverOLED<SSH1106_128x64> display;  
//DaisyHardware hw;  
extern Oscillator osc[16];  
//int encoderValue = 0;  
  
void Waveform() {  
  // Clear and go to Waveform submenu  
  display.clear();  
  display.setCursor(0, 0);  
  display.print("Waveform");  
  display.setCursor(0, 1);  
  display.print("1. Sawtooth");  
  display.setCursor(0, 2);  
  display.print("2. Sine");  
  display.setCursor(0, 3);  
  display.print("3. Triangle");  
  display.setCursor(0, 4);  
  display.print("4. Square");  
  display.setCursor(0, 5);  
  display.print("Back");  
  display.update();  
  
// Highlight selected row  
  display.setCursor(0, 1);  
  display.invertText(true);  
  display.print("1. Sawtooth");  
  display.invertText(false);  
  display.update();  
  
  // Wait for user to select waveform  
  while (true) {  
  synth_loop();  
  hw.DebounceControls();  
  
  if (hw.encoder.Increment() != 0) {  
  if (hw.encoder.Increment() > 0) {  
    encoderValue++;  
  } else {  
    encoderValue--;  
  }  
 // Update highlighted row  
  display.clear();  
  display.setCursor(0, 0);  
  display.print("Waveform");  
  display.setCursor(0, 1);  
  display.print("1. Sawtooth");  
  display.setCursor(0, 2);  
  display.print("2. Sine");  
  display.setCursor(0, 3);  
  display.print("3. Triangle");  
  display.setCursor(0, 4);  
  display.print("4. Square");  
  display.setCursor(0, 5);  
  display.print("Back");  
  display.update();  
  
// Highlight selected row  
  display.setCursor(0, encoderValue + 1);  
  display.invertText(true);  
  if (encoderValue == 0) {  
    display.print("1. Sawtooth");  
  } else if (encoderValue == 1) {  
    display.print("2. Sine");  
  } else if (encoderValue == 2) {  
    display.print("3. Triangle");  
  } else if (encoderValue == 3) {  
    display.print("4. Square");  
  } else if (encoderValue == 4) {  
    display.print("Back");  
  }  
  display.invertText(false);  
  display.update();  
  }  
  
  if (hw.encoder.RisingEdge()) {  
// Select waveform  
  if (encoderValue == 0) {  
    for (int i = 0; i < 12; i++) {  
    osc[i].SetWaveform(Oscillator::WAVE_SAW);  
    }  
  } else if (encoderValue == 1) {  
    for (int i = 0; i < 12; i++) {  
    osc[i].SetWaveform(Oscillator::WAVE_SIN);  
    }  
  } else if (encoderValue == 2) {  
    for (int i = 0; i < 12; i++) {  
    osc[i].SetWaveform(Oscillator::WAVE_TRI);  
    }  
  } else if (encoderValue == 3) {  
    for (int i = 0; i < 12; i++) {  
    osc[i].SetWaveform(Oscillator::WAVE_SQUARE);  
    }  
  } else if (encoderValue == 4) {  
// Go back to main menu  
   display.clear();  
    display.setCursor(0, 0);  
    display.print("MENU");  
    display.setCursor(0, 1);  
    display.print("Waveform");  
    display.setCursor(0, 2);  
    display.print("Envelope");  
    display.update();  
    break;  
  }  
  }  
  }  
}
