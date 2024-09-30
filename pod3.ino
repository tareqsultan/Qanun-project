#include "DaisyDuino.h"  
  
DaisyHardware pod;  
Oscillator osc;  
AdEnv env;  
ReverbSc rev;  
  
void setup() {  
  pod = DAISY.init(DAISY_POD, AUDIO_SR_48K);  
  osc.Init(DAISY.get_samplerate());  
  env.Init(DAISY.get_samplerate());  
  env.SetTime(ADENV_SEG_ATTACK, 0.01);  
  env.SetTime(ADENV_SEG_DECAY, 0.2);  
  env.SetMax(1);  
  env.SetMin(0);  
  osc.SetWaveform(Oscillator::WAVE_SAW);  
  osc.SetAmp(1);  
  rev.Init(DAISY.get_samplerate());  
  DAISY.begin(AudioCallback);  
}  
  
void AudioCallback(float **in, float **out, size_t size) {  
  pod.ProcessDigitalControls();  
  float dryl, dryr, wetl, wetr, sendl, sendr;  
  for (size_t i = 0; i < size; i += 2) {  
  dryl = osc.Process() * env.Process();  
  dryr = dryl;  
  sendl = dryl * 0.5;  
  sendr = dryr * 0.5;  
  rev.Process(sendl, sendr, &wetl, &wetr);  
  out[0][i] = dryl + wetl;  
  out[1][i] = dryr + wetr;  
  }  
}  
  
void loop() {  
  pod.DebounceControls();  
  if (pod.buttons[0].RisingEdge()) {  
  env.Trigger();  
  }  
  int potValue1 = analogRead(PIN_POD_POT_1);  
  float note = map(potValue1, 0, 1023, 60, 72); // map pot value to note range (C4 to C5)  
  osc.SetFreq(mtof(note));  
  
  int potValue2 = analogRead(PIN_POD_POT_2);  
float reverbAmount = (float)potValue2 / 1023.0; // map pot value to reverb amount (0 to 1)  
rev.SetFeedback(reverbAmount);
  
  
  pod.leds[0].Set(env.IsRunning(), 0, 0);  
}
