#include "DaisyDuino.h"  
#include "Adafruit_MPR121.h"  
#include "NoteFrequencies.ino"  
#include "Synthesizer.h"  

float attackTime;  
float decayTime;

DaisyHardware pod;  
AdEnv env[16];  
Oscillator osc[16];  
ReverbSc rev;
extern AdEnv env[16];


Adafruit_MPR121 cap = Adafruit_MPR121();  
Adafruit_MPR121 cap2 = Adafruit_MPR121();  
uint16_t lasttouched = 0;  
uint16_t currtouched = 0; 
uint16_t currtouched2 = 0;
uint16_t lasttouched2 = 0;    
  
// MUX pins  
int s0 = 9;  
int s1 = 8;  
int s2 = 7;  
int Z_pin = 7;  
int Z_pin1 = A1;  
  
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
  int val = analogRead(Z_pin);  
  int freqIndex = map(val, 0, 1023, channel * 3, (channel * 3) + 5);  
  float freq = noteFreq[0][freqIndex];  
  return freq;  
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
  int val = analogRead(Z_pin1);  
  int freqIndex = map(val, 0, 1023, (9 * 3) + (channel * 3), (9 * 3) + (channel * 3) + 5);  
  float freq = noteFreq[0][freqIndex];  
  return freq;  
}

/*
void AudioCallback(float **in, float **out, size_t size) {  
  for (size_t i = 0; i < size; i++) {  
  float sig = 0;  
  for (int j = 0; j < 16; j++) {  
  sig += osc[j].Process() * env[j].Process();  
  }  
  out[0][i] = sig;  
  out[1][i] = sig;  
  }  
}  
*/
void AudioCallback(float **in, float **out, size_t size) {  
  for (size_t i = 0; i < size; i++) {  
  float sig = 0;  
  for (int j = 0; j < 16; j++) {  
  sig += osc[j].Process() * env[j].Process();  
  }  
  float reverbSig;  
  rev.Process(sig, &reverbSig);  
  sig = 0.5f * sig + 0.5f * reverbSig;  
  out[0][i] = sig;  
  out[1][i] = sig;  
  }  
}

 
void synth_setup() {  
  pod = DAISY.init(DAISY_POD, AUDIO_SR_48K);  
  
  // Initialize envelope generators and oscillators  
  for (int i = 0; i < 16; i++) {  
  env[i].Init(DAISY.get_samplerate());  
  osc[i].Init(DAISY.get_samplerate());  
  env[i].SetTime(ADENV_SEG_ATTACK, 0.01);  
  env[i].SetTime(ADENV_SEG_DECAY, 0.2);  
  env[i].SetMax(1);  
  env[i].SetMin(0);  
  hw.leds[0].Set(false, false, false); // Initialize LED to off  
  hw.leds[1].Set(false, false, false); // Initialize LED to off 
  // Update waveform of oscillator based on submenu selection  
  if (submenuOption == 0) {  
  osc[i].SetWaveform(Oscillator::WAVE_SAW);  
  } else if (submenuOption == 1) {  
  osc[i].SetWaveform(Oscillator::WAVE_SIN);  
  } else if (submenuOption == 2) {  
  osc[i].SetWaveform(Oscillator::WAVE_TRI);  
  } else if (submenuOption == 3) {  
  osc[i].SetWaveform(Oscillator::WAVE_SQUARE);  
  }  
  osc[i].SetAmp(1);  
  }  
  
  //DAISY.begin(AudioCallback);  
  
  // Initialize MPR121  
  if (!cap.begin(0x5A)) {  
  Serial.println("MPR121 not found, check wiring?");  
  while (1);  
  }  
  //Serial.println("MPR121 found!");  
  if (!cap2.begin(0x5B)) {  
   //Serial.setCursor(0, 2);  
   Serial.println("MPR121 2 not found");  
   while (1);  
  }
  ; 
  rev.Init(DAISY.get_samplerate());  
  rev.SetLpFreq(18000.0f);  
  rev.SetFeedback(0.05f);  
  DAISY.begin(AudioCallback);  
  
  // Initialize MUX pins  
  pinMode(s0, OUTPUT);  
  pinMode(s1, OUTPUT);  
  pinMode(s2, OUTPUT);  
  digitalWrite(s0, LOW);  
  digitalWrite(s1, LOW);  
  digitalWrite(s2, LOW);  
}

void UpdateKnobs() {  
  attackTime = (float)analogRead(PIN_POD_POT_1) / 1023.0f * 0.99f + 0.01f;  
  decayTime = (float)analogRead(PIN_POD_POT_2) / 1023.0f * 0.8f + 0.2f;  
  
  for (int i = 0; i < 16; i++) {  
  env[i].SetTime(ADENV_SEG_ATTACK, attackTime);  
  env[i].SetTime(ADENV_SEG_DECAY, decayTime);  
  }  
}
  
void synth_loop() {  
  pod.DebounceControls();  
 
  // Get the currently touched pads  
  currtouched = cap.touched();  
  currtouched2 = cap2.touched();  
  
  // Check if any key is touched  
  for (int i = 0; i < 12; i++) {  
  if ((currtouched & _BV(i)) && !(lasttouched & _BV(i))) {  
  env[i].Trigger();  
  }  
  }  
  for (int i = 0; i < 4; i++) {  
  if ((currtouched2 & _BV(i)) && !(lasttouched2 & _BV(i))) {  
  env[i+12].Trigger();  
  }  
  } 
  
  // Read pot values and set frequencies  
  for (int i = 0; i < 16; i++) {  
  float freq;  
  if (i < 8) {  
  freq = readMux1(i); // Read from MUX1  
  } else {  
  freq = readMux2(i-8); // Read from MUX2  
  delay(1); // Add a small delay to ensure MUX2 is properly addressed  
  }  
  osc[i].SetFreq(freq); // Set frequency of oscillator  
  }  
  
  // Update attack and decay times  
  UpdateKnobs();  
  
  // reset our state  
  lasttouched = currtouched; 
  lasttouched2 = currtouched2;   

 // Envelope();
 
}


