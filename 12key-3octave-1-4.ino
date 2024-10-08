#include "DaisyDuino.h"  
#include "Adafruit_MPR121.h"  
 float noteFreq[1][96] = {  
  // Low octave (Octave 2)  
  65.41, 67.35, 69.30, 71.36, 73.42, 75.60, 77.78, 80.09,  
  82.41, 84.86, 87.31, 89.90, 92.50, 95.25, 98.00, 100.91,  
  103.83, 106.91, 110.00, 113.27, 116.54, 120.00, 123.47, 127.14,  
  // Middle octave  
  130.81, 134.70, 138.59, 142.48, 146.83, 151.22, 155.56, 160.95,  
  164.81, 170.20, 174.61, 180.02, 185.00, 190.39, 196.00, 202.41,  
  207.65, 213.86, 220.00, 226.11, 233.08, 240.05, 246.94, 254.83,  
  // High octave  
  261.63, 269.40, 277.18, 285.42, 293.66, 302.40, 311.13, 320.38,  
  329.63, 339.43, 349.23, 359.61, 369.99, 380.69, 392.00, 403.65,  
  415.30, 427.47, 440.00, 453.58, 466.16, 480.37, 493.88, 508.53,  
  // High octave (Octave 5)  
  523.25, 538.81, 554.37, 570.85, 587.33, 604.79, 622.25, 640.75,  
  659.25, 678.85, 698.46, 719.22, 739.99, 761.99, 783.99, 807.30,  
  830.61, 855.30, 880.00, 906.16, 932.33, 960.05, 987.77, 1017.13  
};









DaisyHardware pod;  
AdEnv env[12];  
Oscillator osc[12];  
  
Adafruit_MPR121 cap = Adafruit_MPR121();  
  
uint16_t lasttouched = 0;  
uint16_t currtouched = 0;  
  
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
  return val;  
}  
  
int readMux2(int channel) {  
  int controlPin[] = {s0, s1, s2};  
  int muxChannel[4][3] = {  
  {0, 0, 0}, // channel 0  
  {1, 0, 0}, // channel 1  
  {0, 1, 0}, // channel 2  
  {1, 1, 0}, // channel 3  
  };  
  
  for (int i = 0; i < 3; i++) {  
  digitalWrite(controlPin[i], muxChannel[channel][i]);  
  }  
  int val = analogRead(Z_pin1);  
  return val;  
}

  
void AudioCallback(float **in, float **out, size_t size) {  
  for (size_t i = 0; i < size; i++) {  
  float sig = 0;  
  for (int j = 0; j < 12; j++) {  
  sig += osc[j].Process() * env[j].Process();  
  }  
  out[0][i] = sig;  
  out[1][i] = sig;  
  }  
}  
  
void setup() {  
  pod = DAISY.init(DAISY_POD, AUDIO_SR_48K);  
  
  // Initialize envelope generators and oscillators  
  for (int i = 0; i < 12; i++) {  
  env[i].Init(DAISY.get_samplerate());  
  osc[i].Init(DAISY.get_samplerate());  
  env[i].SetTime(ADENV_SEG_ATTACK, 0.01);  
  env[i].SetTime(ADENV_SEG_DECAY, 0.2);  
  env[i].SetMax(1);  
  env[i].SetMin(0);  
  osc[i].SetWaveform(Oscillator::WAVE_SAW);  
  osc[i].SetAmp(1);  
  }  
  
  DAISY.begin(AudioCallback);  
  
  // Initialize MPR121  
  if (!cap.begin(0x5A)) {  
  Serial.println("MPR121 not found, check wiring?");  
  while (1);  
  }  
  Serial.println("MPR121 found!");  
  
  // Initialize MUX pins  
  pinMode(s0, OUTPUT);  
  pinMode(s1, OUTPUT);  
  pinMode(s2, OUTPUT);  
  digitalWrite(s0, LOW);  
  digitalWrite(s1, LOW);  
  digitalWrite(s2, LOW);  
}  
  
void loop() {  
  pod.DebounceControls();  
  
  // Get the currently touched pads  
  currtouched = cap.touched();  
  
  // Check if any key is touched  
  for (int i = 0; i < 12; i++) {  
  if ((currtouched & _BV(i)) && !(lasttouched & _BV(i))) {  
  env[i].Trigger();  
  }  
  }  
  
  // Read pot values and set frequencies  
for (int i = 0; i < 12; i++) {  
  int potValue;  
  if (i < 8) {  
  potValue = readMux1(i); // Read from MUX1  
  } else {  
  potValue = readMux2(i-8); // Read from MUX2  
 // Add a small delay to ensure MUX2 is properly addressed  
  delay(1);  
  }  
 // float note = map(potValue, 0, 1023, 36, 72);  
  //osc[i].SetFreq(mtof(note));  
//}
int index = map(potValue, 0, 1023, 0, 96); // Map pot value to index in noteFreq array  
float freq = noteFreq[0][index]; // Get frequency from noteFreq array  
osc[i].SetFreq(freq); // Set frequency of oscillator

}


  
  // reset our state  
  lasttouched = currtouched;  
}
