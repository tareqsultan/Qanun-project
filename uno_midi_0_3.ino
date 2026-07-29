#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // RX=2، TX=3 (لشيلد الصوت VS1053)

byte resetMIDI = 4; 

const int fsrPinUp = A1;        
const int sliderPinFull = A2;   

int lastPitchBendValue = 8192; 
const int deadZoneFSR = 95;     
const int maxTouchFSR = 550;    

const int sliderCenter = 512;
const int sliderDeadZone = 32; 

float filteredFSRUp = 0.0;
float filteredSlider = 512.0; 
const float filterFactor = 0.25; 

unsigned long lastFSRCheck = 0;
const unsigned long fsrInterval = 10; 

const int MAX_FSR_BEND_RANGE = 2048; 

void setup() {
  // الاتصال المباشر مع Teensy عبر TX (Pin 1)
  Serial.begin(31250);
  
  // الاتصال بشيلد الصوت عبر D2/D3
  mySerial.begin(31250);

  pinMode(resetMIDI, OUTPUT);
  digitalWrite(resetMIDI, LOW);
  delay(100);
  digitalWrite(resetMIDI, HIGH);
  delay(100);

  talkMIDI(0xB0, 0x07, 120); 
  talkMIDI(0xC0, 0, 0);      

  setPitchBendRange(0, 2); 
  setPitchBendRange(1, 2); 
  
  sendPitchBend(8192, 0);  
  
  filteredFSRUp = analogRead(fsrPinUp);
  filteredSlider = analogRead(sliderPinFull);
}

void loop() {
  // 1️⃣ قراءة أي نغمات قادمة من Teensy وتوجيهها لشيلد الصوت
  while (Serial.available() > 0) {
    byte midiByte = Serial.read();
    mySerial.write(midiByte); 
  }

  // 2️⃣ قراءة حساس الضغط والسلايدر باستمرار وإرسالهما لـ Teensy والـ Shield
  if (millis() - lastFSRCheck >= fsrInterval) {
    lastFSRCheck = millis();

    int rawUp = analogRead(fsrPinUp);
    int rawSlider = analogRead(sliderPinFull);

    int offsetFSR = 0;
    if (rawUp >= deadZoneFSR) {
      filteredFSRUp = (rawUp * filterFactor) + (filteredFSRUp * (1.0 - filterFactor));
      int cleanUp = (int)filteredFSRUp;
      if (cleanUp > maxTouchFSR) cleanUp = maxTouchFSR;
      offsetFSR = map(cleanUp, deadZoneFSR, maxTouchFSR, 0, MAX_FSR_BEND_RANGE);
    } else {
      filteredFSRUp = rawUp;
    }

    int offsetSlider = 0;
    filteredSlider = (rawSlider * filterFactor) + (filteredSlider * (1.0 - filterFactor));
    int cleanSlider = (int)filteredSlider;

    if (cleanSlider < (sliderCenter - sliderDeadZone)) {
      offsetSlider = map(cleanSlider, 0, (sliderCenter - sliderDeadZone), -8192, 0);
    } 
    else if (cleanSlider > (sliderCenter + sliderDeadZone)) {
      offsetSlider = map(cleanSlider, (sliderCenter + sliderDeadZone), 1023, 0, 8191);
    } 
    else {
      offsetSlider = 0;
    }

    int targetPitchBend = 8192 + offsetFSR + offsetSlider;
    if (targetPitchBend > 16383) targetPitchBend = 16383;
    if (targetPitchBend < 0) targetPitchBend = 0;

    if (abs(targetPitchBend - lastPitchBendValue) > 15 || (targetPitchBend == 8192 && lastPitchBendValue != 8192)) {
      lastPitchBendValue = targetPitchBend;
      sendPitchBend(targetPitchBend, 0); 
    }
  }
}

void talkMIDI(byte cmd, byte data1, byte data2) {
  mySerial.write(cmd);
  mySerial.write(data1);
  if ((cmd & 0xF0) <= 0xB0) {
    mySerial.write(data2);
  }
}

void sendPitchBend(int value, byte channel) {
  byte lsb = value & 0x7F;        
  byte msb = (value >> 7) & 0x7F; 
  byte cmd = 0xE0 | (channel & 0x0F); 

  // إرسال لشيلد الصوت التناظري
  mySerial.write(cmd); 
  mySerial.write(lsb);
  mySerial.write(msb);

  // 🎯 إرسال عبر TX المباشر إلى Teensy ليقوم بتحويلها إلى Ableton!
  Serial.write(cmd);   
  Serial.write(lsb);
  Serial.write(msb);
}

void setPitchBendRange(byte channel, byte semitones) {
  byte statusByte = 0xB0 | (channel & 0x0F);
  talkMIDI(statusByte, 0x65, 0x00); 
  talkMIDI(statusByte, 0x64, 0x00); 
  talkMIDI(statusByte, 0x06, semitones); 
  talkMIDI(statusByte, 0x26, 0x00);      
  talkMIDI(statusByte, 0x65, 0x7F);
  talkMIDI(0xB0, 0x64, 0x7F);
}