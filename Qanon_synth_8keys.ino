
// /////////////////TEEENSY 4.1 QANON V 1.0 ////////////////////
//                                                            //
//                        knob8---------------------======    //    
//                     knob7------------------------======    //
//                  knob6---------------------------======    //
//               knob5------------------------------======    //
//           knob4----------------------------------======    //
//        knob3-------------------------------------======    //
//     knob2----------------------------------------======    //
//  knob1-------------------------------------------======    //
//                     BY tareQsultan                         //
////////////////////////////////////////////////////////////////
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Bounce.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform4; //xy=71.58963394165039,124.4444465637207
AudioSynthWaveform       waveform3; //xy=71.54178237915039,165.66189193725586
AudioSynthWaveform       waveform8; //xy=73.10723876953125,575.1654033660889
AudioSynthWaveform       waveform2; //xy=74.76400375366211,267.43966484069824
AudioSynthWaveform       waveform7; //xy=74.51057434082031,526.3828983306885
AudioSynthWaveform       waveform1;      //xy=76.31955337524414,212.32854843139648
AudioSynthWaveform       waveform6; //xy=77.73279571533203,481.91067695617676
AudioSynthWaveform       waveform5; //xy=79.28834533691406,426.799560546875
AudioEffectEnvelope      envelope1;      //xy=226.31957244873047,213.43966674804688
AudioEffectEnvelope      envelope2; //xy=228.31952285766602,267.3285713195801
AudioEffectEnvelope      envelope4; //xy=229.589599609375,125.44446754455566
AudioEffectEnvelope      envelope3; //xy=229.54175186157227,165.55078125
AudioEffectEnvelope      envelope5; //xy=229.2883644104004,427.9106788635254
AudioEffectEnvelope      envelope6; //xy=231.28831481933594,481.7995834350586
AudioEffectEnvelope      envelope8; //xy=231.10720443725586,576.1654243469238
AudioEffectEnvelope      envelope7; //xy=232.5105438232422,526.2717876434326
AudioMixer4              mixer1;         //xy=425.4306831359863,188.43967056274414
AudioMixer4              mixer3; //xy=428.39948654174805,500.41067123413086
AudioMixer4              mixer2;         //xy=612.6528854370117,298.2174377441406
AudioOutputI2S           i2s2;           //xy=774.0973129272461,293.3285160064697
AudioConnection          patchCord1(waveform4, envelope4);
AudioConnection          patchCord2(waveform3, envelope3);
AudioConnection          patchCord3(waveform8, envelope8);
AudioConnection          patchCord4(waveform2, envelope2);
AudioConnection          patchCord5(waveform7, envelope7);
AudioConnection          patchCord6(waveform1, envelope1);
AudioConnection          patchCord7(waveform6, envelope6);
AudioConnection          patchCord8(waveform5, envelope5);
AudioConnection          patchCord9(envelope1, 0, mixer1, 2);
AudioConnection          patchCord10(envelope2, 0, mixer1, 3);
AudioConnection          patchCord11(envelope4, 0, mixer1, 0);
AudioConnection          patchCord12(envelope3, 0, mixer1, 1);
AudioConnection          patchCord13(envelope5, 0, mixer3, 0);
AudioConnection          patchCord14(envelope6, 0, mixer3, 1);
AudioConnection          patchCord15(envelope8, 0, mixer3, 3);
AudioConnection          patchCord16(envelope7, 0, mixer3, 2);
AudioConnection          patchCord17(mixer1, 0, mixer2, 0);
AudioConnection          patchCord18(mixer3, 0, mixer2, 1);
AudioConnection          patchCord19(mixer2, 0, i2s2, 0);
AudioConnection          patchCord20(mixer2, 0, i2s2, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=446.27339935302734,66.84389686584473
// GUItool: end automatically generated code

///////////////////////////////////////////////////////////////
Bounce button0 = Bounce(0, 15);
Bounce button1 = Bounce(1, 15);  // 15 = 15 ms debounce time
Bounce button2 = Bounce(2, 15);
Bounce button3 = Bounce(3, 15);
Bounce button4 = Bounce(4, 15);
Bounce button5 = Bounce(5, 15); 
Bounce button6 = Bounce(6, 15);
Bounce button7 = Bounce(9, 15);    // change pin for audio shield 
Bounce button8 = Bounce(25, 15);   // change pin for audio shield 


void setup() {
  Serial.begin(9600);
  AudioMemory(20);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.32);
  
  pinMode(0, INPUT_PULLUP);//waveform_type

  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(25, INPUT_PULLUP);
  //////////////////////////////////////////////////
  mixer1.gain(0, 0.15);
  mixer1.gain(1, 0.0);
  mixer1.gain(2, 0.0);
  mixer1.gain(3, 0.0);
  mixer2.gain(0, 0.15);
  mixer2.gain(1, 0.15);
  mixer2.gain(2, 0.0);
  mixer2.gain(3, 0.0);
  mixer3.gain(0, 0.15);
  mixer3.gain(1, 0.0);
  mixer3.gain(2, 0.0);
  mixer3.gain(3, 0.0);
//////////////////////////////////waveform/////////////////////////////////////////
  waveform1.begin(WAVEFORM_SAWTOOTH);
  waveform1.amplitude(0.75);
  waveform1.frequency(50);
  waveform1.pulseWidth(0.15);

  waveform2.begin(WAVEFORM_SAWTOOTH);
  waveform2.amplitude(0.75);
  waveform2.frequency(50);
  waveform2.pulseWidth(0.15);

  waveform3.begin(WAVEFORM_SAWTOOTH);
  waveform3.amplitude(0.75);
  waveform3.frequency(50);
  waveform3.pulseWidth(0.15);

  waveform4.begin(WAVEFORM_SAWTOOTH);
  waveform4.amplitude(0.75);
  waveform4.frequency(50);
  waveform4.pulseWidth(0.15);

  waveform5.begin(WAVEFORM_SAWTOOTH);
  waveform5.amplitude(0.75);
  waveform5.frequency(50);
  waveform5.pulseWidth(0.15);

  waveform6.begin(WAVEFORM_SAWTOOTH);
  waveform6.amplitude(0.75);
  waveform6.frequency(50);
  waveform6.pulseWidth(0.15);

  waveform7.begin(WAVEFORM_SAWTOOTH);
  waveform7.amplitude(0.75);
  waveform7.frequency(50);
  waveform7.pulseWidth(0.15);

  waveform8.begin(WAVEFORM_SAWTOOTH);
  waveform8.amplitude(0.75);
  waveform8.frequency(50);
  waveform8.pulseWidth(0.15);

  ///////////////////////////////Envelope////////////////////////////////////////
  envelope1.attack(10);
  envelope1.hold(10);
  envelope1.decay(25);
  envelope1.sustain(0.4);
  envelope1.release(500);

  envelope2.attack(10);
  envelope2.hold(10);
  envelope2.decay(25);
  envelope2.sustain(0.4);
  envelope2.release(500);

  envelope3.attack(10);
  envelope3.hold(10);
  envelope3.decay(25);
  envelope3.sustain(0.4);
  envelope3.release(500);

  envelope4.attack(10);
  envelope4.hold(10);
  envelope4.decay(25);
  envelope4.sustain(0.4);
  envelope4.release(500);

  envelope5.attack(10);
  envelope5.hold(10);
  envelope5.decay(25);
  envelope5.sustain(0.4);
  envelope5.release(500);

  envelope6.attack(10);
  envelope6.hold(10);
  envelope6.decay(25);
  envelope6.sustain(0.4);
  envelope6.release(500);

  envelope7.attack(10);
  envelope7.hold(10);
  envelope7.decay(25);
  envelope7.sustain(0.4);
  envelope7.release(500);

  envelope8.attack(10);
  envelope8.hold(10);
  envelope8.decay(25);
  envelope8.sustain(0.4);
  envelope8.release(500);

}
//////////////////////////waveform_type///////////////////////////////
int waveform_type = WAVEFORM_SAWTOOTH;
int waveform_type1 = WAVEFORM_SAWTOOTH;
int waveform_type2 = WAVEFORM_SAWTOOTH;
int waveform_type3 = WAVEFORM_SAWTOOTH;
int waveform_type4 = WAVEFORM_SAWTOOTH;
int waveform_type5 = WAVEFORM_SAWTOOTH;
int waveform_type6 = WAVEFORM_SAWTOOTH;
int waveform_type7 = WAVEFORM_SAWTOOTH;

///////////////////////////////////////////////////////////
int mixer1_setting = 0;
int mixer2_setting = 0;
int mixer3_setting = 0;

elapsedMillis timeout = 0;
bool mixer2_envelope = false;

void loop() {
  button0.update();
  button1.update();
  button2.update();
  button3.update();
  button4.update(); 
  button5.update();
  button6.update();
  button7.update();
  button8.update();

  // Left changes the type of control waveform
  if (button0.fallingEdge()) {
    Serial.print("Control waveform: ");
    if (waveform_type == WAVEFORM_SAWTOOTH) {
      waveform_type = WAVEFORM_SINE; 
      Serial.println("Sine");
    } else if (waveform_type == WAVEFORM_SINE) {
      waveform_type = WAVEFORM_SQUARE;
      
      Serial.println("Square");
    } else if (waveform_type == WAVEFORM_SQUARE) {
      waveform_type = WAVEFORM_TRIANGLE; 
      Serial.println("Triangle");
    } else if (waveform_type == WAVEFORM_TRIANGLE) {
      waveform_type = WAVEFORM_PULSE; 
      Serial.println("Pulse");
    } else if (waveform_type == WAVEFORM_PULSE) {
      waveform_type = WAVEFORM_SAWTOOTH;  
      Serial.println("Sawtooth");
    }
    waveform1.begin(waveform_type);
  }

  // Left changes the type of control waveform
  if (button0.fallingEdge()) {
    if (waveform_type1 == WAVEFORM_SAWTOOTH) { 
      waveform_type1 = WAVEFORM_SINE;
    } else if (waveform_type1 == WAVEFORM_SINE) { 
      waveform_type1 = WAVEFORM_SQUARE;
    } else if (waveform_type1 == WAVEFORM_SQUARE) {
      waveform_type1 = WAVEFORM_TRIANGLE;
    } else if (waveform_type1 == WAVEFORM_TRIANGLE) {
      waveform_type1 = WAVEFORM_PULSE;
    } else if (waveform_type1 == WAVEFORM_PULSE) { 
      waveform_type1 = WAVEFORM_SAWTOOTH;
    }
    
    waveform2.begin(waveform_type1);
  }

  // Left changes the type of control waveform
  if (button0.fallingEdge()) {
    if (waveform_type2 == WAVEFORM_SAWTOOTH) { 
      waveform_type2 = WAVEFORM_SINE;
    } else if (waveform_type2 == WAVEFORM_SINE) { 
      waveform_type2 = WAVEFORM_SQUARE;
    } else if (waveform_type2 == WAVEFORM_SQUARE) {
      waveform_type2 = WAVEFORM_TRIANGLE;
    } else if (waveform_type2 == WAVEFORM_TRIANGLE) {
      waveform_type2 = WAVEFORM_PULSE;
    } else if (waveform_type2 == WAVEFORM_PULSE) { 
      waveform_type2 = WAVEFORM_SAWTOOTH;
    }
    
    waveform3.begin(waveform_type2);
  }

// Left changes the type of control waveform
  if (button0.fallingEdge()) {
    if (waveform_type3 == WAVEFORM_SAWTOOTH) { 
      waveform_type3 = WAVEFORM_SINE;
    } else if (waveform_type3 == WAVEFORM_SINE) { 
      waveform_type3 = WAVEFORM_SQUARE;
    } else if (waveform_type3 == WAVEFORM_SQUARE) {
      waveform_type3 = WAVEFORM_TRIANGLE;
    } else if (waveform_type3 == WAVEFORM_TRIANGLE) {
      waveform_type3 = WAVEFORM_PULSE;
    } else if (waveform_type3 == WAVEFORM_PULSE) { 
      waveform_type3 = WAVEFORM_SAWTOOTH;
    }
    
    waveform4.begin(waveform_type3);
  }
  
// Left changes the type of control waveform
  if (button0.fallingEdge()) {
    if (waveform_type4 == WAVEFORM_SAWTOOTH) { 
      waveform_type4 = WAVEFORM_SINE;
    } else if (waveform_type4 == WAVEFORM_SINE) { 
      waveform_type4 = WAVEFORM_SQUARE;
    } else if (waveform_type4 == WAVEFORM_SQUARE) {
      waveform_type4 = WAVEFORM_TRIANGLE;
    } else if (waveform_type4 == WAVEFORM_TRIANGLE) {
      waveform_type4 = WAVEFORM_PULSE;
    } else if (waveform_type4 == WAVEFORM_PULSE) { 
      waveform_type4 = WAVEFORM_SAWTOOTH;
    }
    
    waveform5.begin(waveform_type4);
  }

  // Left changes the type of control waveform
  if (button0.fallingEdge()) {
    if (waveform_type5 == WAVEFORM_SAWTOOTH) { 
      waveform_type5 = WAVEFORM_SINE;
    } else if (waveform_type5 == WAVEFORM_SINE) { 
      waveform_type5 = WAVEFORM_SQUARE;
    } else if (waveform_type5 == WAVEFORM_SQUARE) {
      waveform_type5 = WAVEFORM_TRIANGLE;
    } else if (waveform_type5 == WAVEFORM_TRIANGLE) {
      waveform_type5 = WAVEFORM_PULSE;
    } else if (waveform_type5 == WAVEFORM_PULSE) { 
      waveform_type5 = WAVEFORM_SAWTOOTH;
    }
    
    waveform6.begin(waveform_type5);
  }

  // Left changes the type of control waveform
  if (button0.fallingEdge()) {
    if (waveform_type6 == WAVEFORM_SAWTOOTH) { 
      waveform_type6 = WAVEFORM_SINE;
    } else if (waveform_type6 == WAVEFORM_SINE) { 
      waveform_type6 = WAVEFORM_SQUARE;
    } else if (waveform_type6 == WAVEFORM_SQUARE) {
      waveform_type6 = WAVEFORM_TRIANGLE;
    } else if (waveform_type6 == WAVEFORM_TRIANGLE) {
      waveform_type6 = WAVEFORM_PULSE;
    } else if (waveform_type6 == WAVEFORM_PULSE) { 
      waveform_type6 = WAVEFORM_SAWTOOTH;
    }
    waveform7.begin(waveform_type6);
  }

  // Left changes the type of control waveform
  if (button0.fallingEdge()) {
    if (waveform_type7 == WAVEFORM_SAWTOOTH) { 
      waveform_type7 = WAVEFORM_SINE;
    } else if (waveform_type7 == WAVEFORM_SINE) { 
      waveform_type7 = WAVEFORM_SQUARE;
    } else if (waveform_type7 == WAVEFORM_SQUARE) {
      waveform_type7 = WAVEFORM_TRIANGLE;
    } else if (waveform_type7 == WAVEFORM_TRIANGLE) {
      waveform_type7 = WAVEFORM_PULSE;
    } else if (waveform_type7 == WAVEFORM_PULSE) { 
      waveform_type7 = WAVEFORM_SAWTOOTH;
    }
    waveform8.begin(waveform_type7);
  }
 
  
  /////////////////////////////BUTTON1///////////////////////
  if (button1.fallingEdge()) {
    //mixer1.gain(0, 0.0);
    mixer1.gain(2, 1.0);
    mixer2_envelope = true;
    timeout = 0;
    envelope1.noteOn();
  }
  if (button1.risingEdge()) {
    envelope1.noteOff();
    timeout = 0;
  }
////////////////////////////BUTTON2///////////////////////
 if (button2.fallingEdge()) {
    //mixer1.gain(0, 0.0);
    mixer1.gain(3, 1.0);
    mixer2_envelope = true;
    timeout = 0;
    envelope2.noteOn();
  }
  if (button2.risingEdge()) {
    envelope2.noteOff();
    timeout = 0;
  }
////////////////////////////BUTTON3///////////////////////
  if (button3.fallingEdge()) {
    //mixer1.gain(0, 0.0);
    mixer1.gain(1, 1.0);
    mixer2_envelope = true;
    timeout = 0;
    envelope3.noteOn();
  }
  if (button3.risingEdge()) {
    envelope3.noteOff();
    timeout = 0;
  }

////////////////////////////BUTTON4///////////////////////
  if (button4.fallingEdge()) {
    //mixer1.gain(0, 0.0);
    mixer1.gain(0, 1.0);
    mixer2_envelope = true;
    timeout = 0;
    envelope4.noteOn();
  }
  if (button4.risingEdge()) {
    envelope4.noteOff();
    timeout = 0;
  }

  ////////////////////////////BUTTON5///////////////////////
  if (button5.fallingEdge()) {
    //mixer1.gain(0, 0.0);
    mixer3.gain(0, 1.0);
    mixer2_envelope = true;
    timeout = 0;
    envelope5.noteOn();
  }
  if (button5.risingEdge()) {
    envelope5.noteOff();
    timeout = 0;
  }
////////////////////////////BUTTON6///////////////////////
  if (button6.fallingEdge()) {
    //mixer1.gain(0, 0.0);
    mixer3.gain(1, 1.0);
    mixer2_envelope = true;
    timeout = 0;
    envelope6.noteOn();
  }
  if (button6.risingEdge()) {
    envelope6.noteOff();
    timeout = 0;
  }

////////////////////////////BUTTON7///////////////////////
  if (button7.fallingEdge()) {
    //mixer1.gain(0, 0.0);
    mixer3.gain(2, 1.0);
    mixer2_envelope = true;
    timeout = 0;
    envelope7.noteOn();
  }
  if (button7.risingEdge()) {
    envelope7.noteOff();
    timeout = 0;
  }
////////////////////////////BUTTON8///////////////////////
  if (button8.fallingEdge()) {
    //mixer1.gain(0, 0.0);
    mixer3.gain(3, 1.0);
    mixer2_envelope = true;
    timeout = 0;
    envelope8.noteOn();
  }
  if (button8.risingEdge()) {
    envelope8.noteOff();
    timeout = 0;
  }
  
  // use the knobs to adjust parameters
  //float knob1 = (float)analogRead(A1) / 1023.0;
  float knob1 = (float)analogRead(A17) / 1023.0;
  float knob2 = (float)analogRead(A16) / 1023.0;
  float knob3 = (float)analogRead(A15) / 1023.0;
  float knob4 = (float)analogRead(A14) / 1023.0;
  float knob5 = (float)analogRead(A0) / 1023.0;
  float knob6 = (float)analogRead(A1) / 1023.0;
  float knob7 = (float)analogRead(A2) / 1023.0;
  float knob8 = (float)analogRead(A3) / 1023.0;
  waveform1.frequency(360 * knob1 + 0.25);
  waveform2.frequency(360 * knob2 + 0.25);
  waveform3.frequency(360 * knob3 + 0.25);
  waveform4.frequency(360 * knob4 + 0.25);
  waveform5.frequency(360 * knob5 + 0.25);
  waveform6.frequency(360 * knob6 + 0.25);
  waveform7.frequency(360 * knob7 + 0.25);
  waveform8.frequency(360 * knob8 + 0.25);
  
}


