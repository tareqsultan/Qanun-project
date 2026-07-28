#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // RX=2، TX=3 (دبابيس الاتصال الفيزيائية بالشيلد)

byte resetMIDI = 4; 
int instrument = 0;

// =========================================================================
// إعدادات الحواسين وملاحقة القناة تلقائياً
// =========================================================================
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
const unsigned long fsrInterval = 5; 

// متغير لتخزين القناة الحالية تلقائياً (الافتراضية القناة 1 وهي index 0)
byte currentChannel = 0; 

const int MAX_FSR_BEND_RANGE = 2048; 

void setup() {
  Serial.begin(31250);
  mySerial.begin(31250);

  pinMode(resetMIDI, OUTPUT);
  digitalWrite(resetMIDI, LOW);
  delay(100);
  digitalWrite(resetMIDI, HIGH);
  delay(100);

  talkMIDI(0xB0, 0x07, 120); 
  talkMIDI(0xC0, 0, 0); 

  // ضبط المدى للقناتين 1 و 2 كضمان عند الإقلاع
  setPitchBendRange(0, 2); // القناة 1
  setPitchBendRange(1, 2); // القناة 2
  
  sendPitchBend(8192, 0);
  
  filteredFSRUp = analogRead(fsrPinUp);
  filteredSlider = analogRead(sliderPinFull);
}

void loop() {
  bool teensySending = false;

  // 1️⃣ تفريغ وتمرير بايتات الميدي القادمة من Teensy واقتناص القناة الحالية
  while (Serial.available() > 0) {
    teensySending = true;
    byte midiByte = Serial.read();
    
    // اقتناص القناة الحالية: عند وجود أمر Note On / Note Off
    if ((midiByte >= 0x90 && midiByte <= 0x9F) || (midiByte >= 0x80 && midiByte <= 0x8F)) {
      currentChannel = midiByte & 0x0F; // استخراج القناة (0 للـ Channel 1، 1 للـ Channel 2)
    }
    
    mySerial.write(midiByte); // تمرير بيانات Teensy فوراً للشيلد دون إعاقة
  }

  // 2️⃣ معالجة سنسر الضغط والسلايدر (خاصة بالقناة 1 فقط!)
  if (!teensySending && (millis() - lastFSRCheck >= fsrInterval)) {
    lastFSRCheck = millis();

    // 🎯 [التعديل المحوري حلال المشكلة]: 
    // نعمل معالجة السنسر فقط إذا كانت القناة الحالية هي القناة 1 (currentChannel == 0).
    // إذا كانت القناة هي 2 (currentChannel == 1) نتجاهل السنسر تماماً لنترك القناة 2 لـ Teensy والربع تون!
    if (currentChannel == 0) {
      int rawUp = analogRead(fsrPinUp);
      int rawSlider = analogRead(sliderPinFull);

      // حساب إزاحة حساس الضغط A1
      int offsetFSR = 0;
      if (rawUp >= deadZoneFSR) {
        filteredFSRUp = (rawUp * filterFactor) + (filteredFSRUp * (1.0 - filterFactor));
        int cleanUp = (int)filteredFSRUp;
        if (cleanUp > maxTouchFSR) cleanUp = maxTouchFSR;
        offsetFSR = map(cleanUp, deadZoneFSR, maxTouchFSR, 0, MAX_FSR_BEND_RANGE);
      } else {
        filteredFSRUp = rawUp;
      }

      // حساب إزاحة السلايدر A2
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

      // إرسال الـ Pitch Bend فقط على القناة 1 عند تغير السنسر
      if (abs(targetPitchBend - lastPitchBendValue) > 15 || (targetPitchBend == 8192 && lastPitchBendValue != 8192)) {
        lastPitchBendValue = targetPitchBend;
        sendPitchBend(targetPitchBend, 0); // إرسال صريح للقناة 1 دائماً
      }
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
  
  mySerial.write(cmd); 
  mySerial.write(lsb);
  mySerial.write(msb);
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