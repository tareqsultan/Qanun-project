#include <FastLED.h>
#include <Trill.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SD.h>
#include <SPI.h>

// =========================================================================
// 1️⃣ إعدادات شاشة OLED
// =========================================================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3D 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool isOledOnline = false;

// =========================================================================
// 2️⃣ إعدادات مصفوفات الليدات (Dual FastLED Configuration)
// =========================================================================
#define MAIN_DATA_PIN    3
#define MAIN_NUM_LEDS    194 
#define MATRIX_WIDTH     18
#define MATRIX_HEIGHT    16
CRGB mainLeds[MAIN_NUM_LEDS];

#define SUB_DATA_PIN     2
#define SUB_NUM_LEDS     23 
CRGB subLeds[SUB_NUM_LEDS];

#define LED_TYPE         WS2812B
#define COLOR_ORDER      GRB
#define BRIGHTNESS       128 

const int16_t XY_Table[MATRIX_HEIGHT][MATRIX_WIDTH] = {
  {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1},
  {17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34},
  {51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, -1},
  {52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, -1, -1},
  {82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, -1, -1, -1},
  {83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, -1, -1, -1, -1},
  {109, 108, 107, 106, 105, 104, 103, 102, 101, 100, 99, 98, 97, -1, -1, -1, -1, -1},
  {110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, -1, -1, -1, -1, -1, -1},
  {133, 132, 131, 130, 129, 128, 127, 126, 125, 124, 123, 122, -1, -1, -1, -1, -1, -1},
  {134, 135, 136, 137, 138, 139, 140, 141, 142, 143, -1, -1, -1, -1, -1, -1, -1, -1},
  {154, 153, 152, 151, 150, 149, 148, 147, 146, 145, -1, -1, -1, -1, -1, -1, -1, -1},
  {156, 157, 158, 159, 160, 161, 162, 163, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {171, 170, 169, 168, 167, 166, 165, 164, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {172, 173, 174, 175, 176, 177, 178, 179, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {186, 185, 184, 183, 182, 181, 180, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {187, 188, 189, 190, 191, 192, 193, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
};

// =========================================================================
// 3️⃣ إعدادات المقابض وعزل القراءات الذكي
// =========================================================================
const int numPots = 16;
const uint8_t potPins[numPots] = {20, 21, 22, 23, 17, 16, 15, 14, 40, 39, 38, 41, 27, 26, 25, 24};
const uint8_t potLedMap[numPots] = {22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7}; 

int lastNoteValues[numPots]; 
int lastHandRaw[numPots];        
bool isHandActive[numPots] = {false}; 

#define BRIGHT_DIM    60   
#define BRIGHT_FULL   150  

// =========================================================================
// 4️⃣ إعدادات حساس اللمس (Trill) ومصفوفات التأخير والـ Toggle
// =========================================================================
Trill trillSensor;
const int numKeys = 16;          
int cMajorScale[numKeys]; 
bool noteStates[numKeys];           
bool isSensorOnline = false;

// 🎯 التعريفات التي كانت مفقودة وتسببت في الخطأ:
bool toggleStates[numKeys] = {false};    
bool isTouchHandled[numKeys] = {false};  

bool pendingNoteOff[numKeys] = {false};
unsigned long noteReleaseTime[numKeys] = {0};

const int touchThreshold = 300;       
const int releaseThreshold = 180;     

unsigned long lastDisplayTime = 0;
const unsigned long displayInterval = 80; 

// =========================================================================
// 📑 تبويب ومصفوفة أسماء آلات الـ General MIDI (128 آلة)
// =========================================================================
const char* instrumentNames[128] = {
  "Acoustic Grand Piano", "Bright A. Piano", "Electric Grand Piano", "Honky-tonk Piano",
  "Electric Piano 1", "Electric Piano 2", "Harpsichord", "Clavi",
  "Celesta", "Glockenspiel", "Music Box", "Vibraphone",
  "Marimba", "Xylophone", "Tubular Bells", "Dulcimer",
  "Drawbar Organ", "Percussive Organ", "Rock Organ", "Church Organ",
  "Reed Organ", "Accordion", "Harmonica", "Tango Accordion",
  "Nylon Guitar", "Steel Guitar", "Jazz Guitar", "Clean Guitar",
  "Muted Guitar", "Overdriven Guitar", "Distortion Guitar", "Guitar Harmonics",
  "Acoustic Bass", "Finger Bass", "Pick Bass", "Fretless Bass",
  "Slap Bass 1", "Slap Bass 2", "Synth Bass 1", "Synth Bass 2",
  "Violin", "Viola", "Cello", "Contrabass",
  "Tremolo Strings", "Pizzicato Strings", "Orchestral Harp", "Timpani",
  "String Ensemble 1", "String Ensemble 2", "SynthStrings 1", "SynthStrings 2",
  "Choir Aahs", "Voice Oohs", "Synth Voice", "Orchestra Hit",
  "Trumpet", "Trombone", "Tuba", "Muted Trumpet",
  "French Horn", "Brass Section", "SynthBrass 1", "SynthBrass 2",
  "Soprano Sax", "Alto Sax", "Tenor Sax", "Baritone Sax",
  "Oboe", "English Horn", "Bassoon", "Clarinet",
  "Piccolo", "Flute", "Recorder", "Pan Flute", "Blown Bottle", "Shakuhachi", "Whistle", "Ocarina",
  "Lead 1 (square)", "Lead 2 (sawtooth)", "Lead 3 (calliope)", "Lead 4 (chiff)",
  "Lead 5 (charang)", "Lead 6 (voice)", "Lead 7 (fifths)", "Lead 8 (bass+lead)",
  "Pad 1 (new age)", "Pad 2 (warm)", "Pad 3 (polysynth)", "Pad 4 (choir)",
  "Pad 5 (bowed)", "Pad 6 (metallic)", "Pad 7 (halo)", "Pad 8 (sweep)",
  "FX 1 (rain)", "FX 2 (soundtrack)", "FX 3 (crystal)", "FX 4 (atmosphere)",
  "FX 5 (brightness)", "FX 6 (goblins)", "FX 7 (echoes)", "FX 8 (sci-fi)",
  "Sitar", "Banjo", "Shamisen", "Koto", "Kalimba", "Bag pipe", "Fiddle", "Shanai",
  "Tinkle Bell", "Agogo", "Steel Drums", "Woodblock", "Taiko Drum", "Melodic Tom",
  "Synth Drum", "Reverse Cymbal", "Guitar Fret Noise", "Breath Noise",
  "Seashore", "Bird Tweet", "Telephone Ring", "Helicopter", "Applause", "Gunshot"
};

// =========================================================================
// 5️⃣ إعدادات أزرار الربع تون، اختيار الصفحات والـ SD Card ودبابيس التعديل
// =========================================================================
#define SELECT_PAGE_PIN 29
#define SAVE_LOAD_PIN   28  
#define PIN_PLUS        11
#define PIN_MINUS       12

const int numPageButtons = 7;
const int buttonPins[numPageButtons] = {4, 5, 6, 7, 8, 9, 10}; 

bool lastButtonStates[numPageButtons] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
const uint8_t buttonLedMap[numPageButtons] = {6, 5, 4, 3, 2, 1, 0}; 

bool padQuarterToneActive[numKeys] = {false}; 
int currentPage = 1; 

bool lastPlusState = HIGH;
bool lastMinusState = HIGH;

int presetNumber = 0;         
int programChangeVal = 1;     
int transposeOffset = 0;      
int microtuneCentValue = -23; 
int noteDurationVal = 1;     
int workingMode = 0;          

bool lastSaveBtnState = HIGH;
bool isSdCardReady = false;

const char* pageNames[7] = {
  "1: PRESET NUM",
  "2: PROGRAM CHANGE",
  "3: TRANSPOSE ALL",
  "4: MICROTUNE CH2",
  "5: NOTE DURATION", 
  "6: CONTROL MODE",
  "7: PAGE 7"
};

// =========================================================================
// 6️⃣ دوال الإضاءة، الشاشة والتحكم بالـ SD Card
// =========================================================================

void updateButtonLEDs() {
  for (int i = 0; i < numPageButtons; i++) {
    bool isGroupActive = false;
    switch(buttonPins[i]) {
      case 4:  isGroupActive = (padQuarterToneActive[0] || padQuarterToneActive[7] || padQuarterToneActive[14]); break;
      case 5:  isGroupActive = (padQuarterToneActive[1] || padQuarterToneActive[8] || padQuarterToneActive[15]); break;
      case 6:  isGroupActive = (padQuarterToneActive[2] || padQuarterToneActive[9]); break; 
      case 7:  isGroupActive = (padQuarterToneActive[3] || padQuarterToneActive[10]); break;
      case 8:  isGroupActive = (padQuarterToneActive[4] || padQuarterToneActive[11]); break;
      case 9:  isGroupActive = (padQuarterToneActive[5] || padQuarterToneActive[12]); break;
      case 10: isGroupActive = (padQuarterToneActive[6] || padQuarterToneActive[13]); break;
    }
    uint8_t physicalLedIndex = buttonLedMap[i];
    subLeds[physicalLedIndex] = isGroupActive ? CRGB::Blue : CRGB::Black;
  }
}

void setMainLineState(int lineIndex, bool turnOn) {
  for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
    int16_t ledIndex = XY_Table[lineIndex][x];
    if (ledIndex != -1) {
      if (turnOn) {
        mainLeds[ledIndex] = padQuarterToneActive[lineIndex] ? CRGB::Blue : CRGB::Green;
      } else {
        mainLeds[ledIndex] = CRGB::Black;
      }
    }
  }
  if (turnOn) {
    int16_t firstLed = XY_Table[lineIndex][0];
    if (firstLed != -1) mainLeds[firstLed] = CRGB::Red; 
  }
}

int centToPitchBend(int cents) {
  int pb = 8192 + (int)(cents * 40.96);
  return constrain(pb, 0, 16383);
}

void sendMicrotuneValue(int cents) {
  int pbVal = centToPitchBend(cents);
  byte lsb = pbVal & 0x7F;
  byte msb = (pbVal >> 7) & 0x7F;
  
  Serial1.write(0xB1); Serial1.write(0x65); Serial1.write(0x00);
  Serial1.write(0xB1); Serial1.write(0x64); Serial1.write(0x00);
  Serial1.write(0xB1); Serial1.write(0x06); Serial1.write(0x02);
  
  usbMIDI.sendPitchBend(pbVal, 2); 
  Serial1.write(0xE1); 
  Serial1.write(lsb);
  Serial1.write(msb);
}

void sendProgramChange(int pVal) {
  usbMIDI.sendProgramChange(pVal - 1, 1); 
  usbMIDI.sendProgramChange(pVal - 1, 2); 
  
  Serial1.write(0xC0); 
  Serial1.write(pVal - 1);
  Serial1.write(0xC1); 
  Serial1.write(pVal - 1);
}

void updateOledDisplay(int keyNum, int noteVal, const char* statusStr) {
  if (!isOledOnline) return;
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0);
  display.print("P:"); display.print(pageNames[currentPage - 1]);
  display.drawFastHLine(0, 11, 128, SSD1306_WHITE);
  
  display.setCursor(0, 18);
  
  if (currentPage == 1) {
    display.print("PRESET: "); display.print(presetNumber);
  } 
  else if (currentPage == 2) {
    int instIndex = constrain(programChangeVal - 1, 0, 127);
    display.print(programChangeVal);
    display.print(":");
    display.print(instrumentNames[instIndex]);
  }
  else if (currentPage == 3) {
    display.print("TRANS: "); display.print(transposeOffset > 0 ? "+" : ""); display.print(transposeOffset);
  }
  else if (currentPage == 4) {
    display.print("TUNE: "); display.print(microtuneCentValue); display.print(" Cent");
  }
  else if (currentPage == 5) {
    display.print("RELEASE: "); display.print(noteDurationVal * 10); display.print(" ms");
  }
  else if (currentPage == 6) {
    display.print("MODE: "); display.print(workingMode == 0 ? "SCALE/TUNE" : "MIDI CC");
  }
  else {
    display.print("KEY: "); display.print(keyNum);
    display.setCursor(64, 18);
    display.print("NOTE: "); display.print(noteVal);
  }
  
  display.setCursor(105, 18);
  display.print(statusStr);
  display.display();
}

void showOledMessage(const char* line1, const char* line2) {
  if (!isOledOnline) return;
  display.clearDisplay();
  display.setCursor(0, 4);
  display.setTextSize(1);
  display.print(line1);
  display.setCursor(0, 18);
  display.print(line2);
  display.display();
}

void savePresetToSD() {
  if (!isSdCardReady) {
    showOledMessage("SD CARD ERROR", "CANNOT SAVE!");
    delay(1000);
    return;
  }
  
  char currentFileName[20];
  snprintf(currentFileName, sizeof(currentFileName), "preset_%d.txt", presetNumber);

  showOledMessage("SD CARD SYSTEM", "SAVING PRESET...");
  
  if (SD.exists(currentFileName)) {
    SD.remove(currentFileName);
  }
  
  File myFile = SD.open(currentFileName, FILE_WRITE);
  if (myFile) {
    for (int i = 0; i < numKeys; i++) {
      myFile.println(padQuarterToneActive[i] ? "1" : "0");
    }
    
    for (int i = 0; i < numPots; i++) {
      myFile.println(cMajorScale[i]);
    }

    myFile.println(presetNumber);
    myFile.println(programChangeVal);
    myFile.println(transposeOffset);
    myFile.println(microtuneCentValue);
    myFile.println(noteDurationVal);
    myFile.println(workingMode);
    
    myFile.close();
    showOledMessage("SD CARD SYSTEM", "SAVE SUCCESS!");
    
    for(int i = 0; i < 7; i++) subLeds[i] = CRGB::Green;
    FastLED.show();
    delay(400);
    
    updateButtonLEDs();
    FastLED.show();
  } else {
    showOledMessage("SD CARD SYSTEM", "WRITE FAILED!");
    delay(1000);
  }
}

void loadPresetFromSD() {
  if (!isSdCardReady) return;
  
  char currentFileName[20];
  snprintf(currentFileName, sizeof(currentFileName), "preset_%d.txt", presetNumber);
  
  if (!SD.exists(currentFileName)) {
    showOledMessage("SD CARD SYSTEM", "NO PRESET FOUND");
    delay(800);
    return;
  }
  
  File myFile = SD.open(currentFileName, FILE_READ);
  if (myFile) {
    int keyIdx = 0;
    while (myFile.available() && keyIdx < numKeys) {
      String line = myFile.readStringUntil('\n');
      line.trim();
      if (line.length() > 0) {
        padQuarterToneActive[keyIdx] = (line == "1");
        keyIdx++;
      }
    }

    int potIdx = 0;
    while (myFile.available() && potIdx < numPots) {
      String line = myFile.readStringUntil('\n');
      line.trim();
      if (line.length() > 0) {
        int savedNote = line.toInt();
        cMajorScale[potIdx] = savedNote;
        lastNoteValues[potIdx] = savedNote;
        
        uint8_t ledIndex = potLedMap[potIdx]; 
        uint8_t noteHue = map(savedNote, 52, 84, 0, 255);
        subLeds[ledIndex] = CHSV(noteHue, 255, BRIGHT_DIM);
      }
      potIdx++;
    }

    if (myFile.available()) {
      myFile.readStringUntil('\n');
    }
    
    if (myFile.available()) {
      programChangeVal = myFile.readStringUntil('\n').toInt(); 
      sendProgramChange(programChangeVal); 
    }
    
    if (myFile.available()) {
      transposeOffset = myFile.readStringUntil('\n').toInt(); 
    }
    
    if (myFile.available()) {
      microtuneCentValue = myFile.readStringUntil('\n').toInt(); 
      sendMicrotuneValue(microtuneCentValue); 
    }

    if (myFile.available()) {
      noteDurationVal = myFile.readStringUntil('\n').toInt(); 
    }

    if (myFile.available()) {
      workingMode = myFile.readStringUntil('\n').toInt(); 
    }
    
    myFile.close();
    
    for (int i = 0; i < numPots; i++) {
      lastHandRaw[i] = analogRead(potPins[i]);
      isHandActive[i] = false; 
    }

    updateButtonLEDs();
    updateOledDisplay(0, 0, "LOADED");
    
    showOledMessage("SD CARD SYSTEM", "LOAD SUCCESS!");
    
    for(int i = 0; i < 7; i++) subLeds[i] = CRGB::Blue;
    FastLED.show(); 
    delay(400);
    
    updateButtonLEDs();
    FastLED.show();
  }
}

void testSubLeds() {
  for(int i = 0; i < SUB_NUM_LEDS; i++) {
    subLeds[i] = CRGB::White;
    FastLED.show();
    delay(10);
  }
  for(int i = 0; i < SUB_NUM_LEDS; i++) {
    subLeds[i] = CRGB::Black;
    FastLED.show();
    delay(5);
  }
}

// =========================================================================
// 7️⃣ دالة التهيئة (Setup)
// =========================================================================
void setup() {
  Serial.begin(115200);
  delay(1000); 

  Serial1.begin(31250); 

  pinMode(SELECT_PAGE_PIN, INPUT_PULLUP);
  pinMode(SAVE_LOAD_PIN, INPUT_PULLUP);
  pinMode(PIN_PLUS, INPUT_PULLUP);
  pinMode(PIN_MINUS, INPUT_PULLUP);
  
  for (int i = 0; i < numPageButtons; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  FastLED.addLeds<LED_TYPE, MAIN_DATA_PIN, COLOR_ORDER>(mainLeds, MAIN_NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, SUB_DATA_PIN, COLOR_ORDER>(subLeds, SUB_NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  
  FastLED.clear();
  FastLED.show();

  testSubLeds(); 

  Wire.begin();
  Wire.setClock(400000); 
  Wire.setTimeout(3000); 

  if(display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    isOledOnline = true;
    showOledMessage("SYSTEM ONLINE", "TRILL MIDI READY");
    delay(800);
  }

  if (SD.begin(BUILTIN_SDCARD)) {
    isSdCardReady = true;
    loadPresetFromSD(); 
  } else {
    showOledMessage("SD CARD SYSTEM", "SD INIT FAILED!");
    delay(1000);
  }

  for (int i = 0; i < numPots; i++) {
    pinMode(potPins[i], INPUT);
    int rawValue = analogRead(potPins[i]);
    lastHandRaw[i] = rawValue; 
    int currentNote = map(rawValue, 0, 1023, 52, 84);
    lastNoteValues[i] = currentNote;
    cMajorScale[i] = currentNote; 
    
    uint8_t ledIndex = potLedMap[i]; 
    uint8_t noteHue = map(currentNote, 52, 84, 0, 255);
    subLeds[ledIndex] = CHSV(noteHue, 255, BRIGHT_DIM); 
  }
  
  currentPage = 1; 
  updateButtonLEDs();
  FastLED.show(); 

  int ret = trillSensor.setup(Trill::TRILL_CRAFT);
  if (ret == 0) {
    isSensorOnline = true;
    trillSensor.setMode(Trill::RAW);         
    trillSensor.setNoiseThreshold(25);       
    trillSensor.setPrescaler(2);             
    trillSensor.setScanSettings(0, 15);      
    trillSensor.updateBaseline();            
  } else {
    isSensorOnline = false;
    showOledMessage("TRILL SENSOR", "DETECT ERROR!");
    delay(1000);
  }

  for (int i = 0; i < numKeys; i++) {
    noteStates[i] = false;
    pendingNoteOff[i] = false;
    toggleStates[i] = false;
    isTouchHandled[i] = false;
  }
  
  sendMicrotuneValue(microtuneCentValue);
  
  updateOledDisplay(0, 0, "IDLE");
}

// =========================================================================
// 8️⃣ الحلقة الرئيسية (Loop)
// =========================================================================
void loop() {
  usbMIDI.read(); 

  // -----------------------------------------------------------------
  // 🎯 استلام إشارات حساس الضغط والسلايدر من Uno وإرسالها لـ Ableton عبر USB
  // -----------------------------------------------------------------
  static byte midiBuffer[3];
  static byte bufIndex = 0;

  while (Serial1.available() > 0) {
    byte inByte = Serial1.read();

    if (inByte >= 0x80) {
      bufIndex = 0;
      midiBuffer[bufIndex++] = inByte;
    } 
    else if (bufIndex > 0) {
      midiBuffer[bufIndex++] = inByte;
      
      // 🎯 منع Pitch Bend أثناء التواجد بالصفحة 6 لعدم التعارض مع Mapping المقابض
      if ((midiBuffer[0] & 0xF0) == 0xE0 && bufIndex == 3) {
        int pbValue = midiBuffer[1] | (midiBuffer[2] << 7);
        byte channel = (midiBuffer[0] & 0x0F) + 1; 
        
        if (currentPage != 6) {
          usbMIDI.sendPitchBend(pbValue, channel); 
        }
        bufIndex = 0; 
      }
      else if ((midiBuffer[0] & 0xF0) == 0xB0 && bufIndex == 3) {
        byte control = midiBuffer[1];
        byte val = midiBuffer[2];
        byte channel = (midiBuffer[0] & 0x0F) + 1;
        
        usbMIDI.sendControlChange(control, val, channel);
        bufIndex = 0;
      }
      else if (bufIndex >= 3) {
        bufIndex = 0; 
      }
    }
  }

  bool needsLedsUpdate = false; 
  int activeKeyForDisplay = -1;
  int activeNoteForDisplay = -1;
  const char* activeStatusForDisplay = "IDLE";
  bool valuesChangedForDisplay = false;

  // -----------------------------------------------------------------
  // [أولاً]: فحص الأزرار الـ + والـ - (Pin 11 & Pin 12)
  // -----------------------------------------------------------------
  bool plusState = (digitalRead(PIN_PLUS) == LOW);
  bool minusState = (digitalRead(PIN_MINUS) == LOW);

  if (plusState != (lastPlusState == LOW)) {
    delay(10); 
    if (plusState) {
      if (currentPage == 1) { 
        if (presetNumber < 99) { presetNumber++; valuesChangedForDisplay = true; }
      } 
      else if (currentPage == 2) { 
        if (programChangeVal < 128) { 
          programChangeVal++; 
          sendProgramChange(programChangeVal); 
          valuesChangedForDisplay = true; 
        }
      } 
      else if (currentPage == 3) { 
        if (transposeOffset < 12) { transposeOffset++; valuesChangedForDisplay = true; }
      } 
      else if (currentPage == 4) { 
        if (microtuneCentValue < 0) { 
          microtuneCentValue++; 
          sendMicrotuneValue(microtuneCentValue); 
          valuesChangedForDisplay = true; 
        }
      }
      else if (currentPage == 5) { 
        if (noteDurationVal < 100) { noteDurationVal++; valuesChangedForDisplay = true; }
      }
      else if (currentPage == 6) { 
        workingMode = (workingMode == 0) ? 1 : 0;
        valuesChangedForDisplay = true;
      }
    }
    lastPlusState = plusState ? LOW : HIGH;
  }

  if (minusState != (lastMinusState == LOW)) {
    delay(10); 
    if (minusState) {
      if (currentPage == 1) {
        if (presetNumber > 0) { presetNumber--; valuesChangedForDisplay = true; }
      } 
      else if (currentPage == 2) {
        if (programChangeVal > 1) { 
          programChangeVal--; 
          sendProgramChange(programChangeVal); 
          valuesChangedForDisplay = true; 
        }
      } 
      else if (currentPage == 3) {
        if (transposeOffset > -12) { transposeOffset--; valuesChangedForDisplay = true; }
      } 
      else if (currentPage == 4) {
        if (microtuneCentValue > -99) { 
          microtuneCentValue--; 
          sendMicrotuneValue(microtuneCentValue); 
          valuesChangedForDisplay = true; 
        }
      }
      else if (currentPage == 5) {
        if (noteDurationVal > 0) { noteDurationVal--; valuesChangedForDisplay = true; }
      }
      else if (currentPage == 6) { 
        workingMode = (workingMode == 0) ? 1 : 0;
        valuesChangedForDisplay = true;
      }
    }
    lastMinusState = minusState ? LOW : HIGH;
  }

  if (valuesChangedForDisplay) {
    activeKeyForDisplay = 0; 
    activeNoteForDisplay = 0;
    activeStatusForDisplay = "CHNG";
  }

  // -----------------------------------------------------------------
  // [ثانياً]: فحص زر التحميل والحفظ (Pin 28 مع زر Pin 29)
  // -----------------------------------------------------------------
  bool saveBtnState = (digitalRead(SAVE_LOAD_PIN) == LOW);
  
  if (saveBtnState != (lastSaveBtnState == LOW)) {
    delay(15); 
    
    if (saveBtnState) { 
      bool isPageBtnHeld = (digitalRead(SELECT_PAGE_PIN) == LOW); 
      
      if (isPageBtnHeld) {
        savePresetToSD();
      } else {
        loadPresetFromSD();
      }
      
      updateButtonLEDs();
      needsLedsUpdate = true;
    }
    lastSaveBtnState = saveBtnState ? LOW : HIGH;
  }

  // -----------------------------------------------------------------
  // [ثالثاً]: فحص الأزرار السبعة (الربع تون الفوري + تغيير الصفحات)
  // -----------------------------------------------------------------
  bool isPageBtnPressed = (digitalRead(SELECT_PAGE_PIN) == LOW); 

  for (int i = 0; i < numPageButtons; i++) {
    bool btnState = (digitalRead(buttonPins[i]) == LOW);
    
    if (btnState != (lastButtonStates[i] == LOW)) {
      delay(15); 
      
      if (btnState) { 
        if (isPageBtnPressed) {
          currentPage = i + 1; 
          
          updateButtonLEDs();
          needsLedsUpdate = true;
          valuesChangedForDisplay = true; 
          activeKeyForDisplay = 0;
          activeNoteForDisplay = 0;
          activeStatusForDisplay = "PAGE";
        } else {
          switch(buttonPins[i]) {
            case 4:
              padQuarterToneActive[0] = !padQuarterToneActive[0];
              padQuarterToneActive[7] = !padQuarterToneActive[7];
              padQuarterToneActive[14] = !padQuarterToneActive[14];
              break;
            case 5:
              padQuarterToneActive[1] = !padQuarterToneActive[1];
              padQuarterToneActive[8] = !padQuarterToneActive[8];
              padQuarterToneActive[15] = !padQuarterToneActive[15];
              break;
            case 6:
              padQuarterToneActive[2] = !padQuarterToneActive[2];
              padQuarterToneActive[9] = !padQuarterToneActive[9];
              break;
            case 7:
              padQuarterToneActive[3] = !padQuarterToneActive[3];
              padQuarterToneActive[10] = !padQuarterToneActive[10];
              break;
            case 8:
              padQuarterToneActive[4] = !padQuarterToneActive[4];
              padQuarterToneActive[11] = !padQuarterToneActive[11];
              break;
            case 9:
              padQuarterToneActive[5] = !padQuarterToneActive[5];
              padQuarterToneActive[12] = !padQuarterToneActive[12];
              break;
            case 10:
              padQuarterToneActive[6] = !padQuarterToneActive[6];
              padQuarterToneActive[13] = !padQuarterToneActive[13];
              break;
          }
          updateButtonLEDs();
          needsLedsUpdate = true;
        }
      }
      lastButtonStates[i] = btnState ? LOW : HIGH;
    }
  }

  // -----------------------------------------------------------------
  // [رابعاً]: قراءة المقابض الـ 16 المفلترة لمنع الاهتزاز
  // -----------------------------------------------------------------
  for (int i = 0; i < numPots; i++) {
    int currentRaw = analogRead(potPins[i]);

    if (abs(currentRaw - lastHandRaw[i]) > 45) {
      isHandActive[i] = true; 
    }

    if (!isHandActive[i]) {
      continue; 
    }

    if (workingMode == 0) {
      int currentNote = map(currentRaw, 0, 1023, 52, 84);

      if (currentNote != lastNoteValues[i]) { 
        lastHandRaw[i] = currentRaw; 
        lastNoteValues[i] = currentNote;
        cMajorScale[i] = currentNote; 

        uint8_t ledIndex = potLedMap[i];
        uint8_t noteHue = map(currentNote, 52, 84, 0, 255);
        
        uint8_t currentBrightness = noteStates[i] ? BRIGHT_FULL : BRIGHT_DIM;
        subLeds[ledIndex] = CHSV(noteHue, 255, currentBrightness);
        
        needsLedsUpdate = true;
        activeKeyForDisplay = i + 1;
        activeNoteForDisplay = currentNote;
        activeStatusForDisplay = "TUNE";
      }
    } 
    else {
      int rawCCValue = map(currentRaw, 0, 1023, 0, 127);
      
      if (abs(rawCCValue - lastNoteValues[i]) >= 2 || (rawCCValue == 0 && lastNoteValues[i] != 0) || (rawCCValue == 127 && lastNoteValues[i] != 127)) { 
        lastHandRaw[i] = currentRaw; 
        lastNoteValues[i] = rawCCValue;
        
        byte ccNumber = 20 + i; 
        usbMIDI.sendControlChange(ccNumber, rawCCValue, 1); 
        
        Serial1.write(0xB0); 
        Serial1.write(ccNumber); 
        Serial1.write(rawCCValue); 

        uint8_t ledIndex = potLedMap[i];
        subLeds[ledIndex] = CHSV(160, 255, map(rawCCValue, 0, 127, 20, 255));
        
        needsLedsUpdate = true;
        activeKeyForDisplay = i + 1;
        activeNoteForDisplay = rawCCValue;
        activeStatusForDisplay = " CC ";
      }
    }
  }

 // -----------------------------------------------------------------
  // [خامساً]: قراءة حساس اللمس (Trill) - نمط العزف الموسيقي الطبيعي
  // -----------------------------------------------------------------
  if (isSensorOnline) {
    trillSensor.requestRawData();
    int pinCounter = 0; 

    while(trillSensor.rawDataAvailable() > 0 && pinCounter < numKeys) {
      int rawTouchValue = trillSensor.rawDataRead(); 
      int originalNote = cMajorScale[pinCounter]; 
      int midiNote = originalNote + transposeOffset; 
      
      uint8_t ledIndex = potLedMap[pinCounter];

      byte targetChannel = padQuarterToneActive[pinCounter] ? 2 : 1;
      byte statusByteOn = (targetChannel == 2) ? 0x91 : 0x90; 
      byte statusByteOff = (targetChannel == 2) ? 0x81 : 0x80; 

      // 1. لحظة اللمس (عزف النغمة ON)
      if (rawTouchValue > touchThreshold) {
        pendingNoteOff[pinCounter] = false; // إلغاء أي أمر إيقاف معلق
        
        if (!noteStates[pinCounter]) {
          if (targetChannel == 2) {
            int pbVal = centToPitchBend(microtuneCentValue);
            byte pbLsb = pbVal & 0x7F;
            byte pbMsb = (pbVal >> 7) & 0x7F;
            
            Serial1.write(0xB1); Serial1.write(0x65); Serial1.write(0x00);
            Serial1.write(0xB1); Serial1.write(0x64); Serial1.write(0x00);
            Serial1.write(0xB1); Serial1.write(0x06); Serial1.write(0x02);
            
            usbMIDI.sendPitchBend(pbVal, 2); 
            Serial1.write(0xE1); 
            Serial1.write(pbLsb);
            Serial1.write(pbMsb);
          }

          usbMIDI.sendNoteOn(midiNote, 105, targetChannel); 
          Serial1.write(statusByteOn); 
          Serial1.write(midiNote);
          Serial1.write(105);  

          setMainLineState(pinCounter, true); 
          
          uint8_t noteHue = map(originalNote, 52, 84, 0, 255);
          subLeds[ledIndex] = CHSV(noteHue, 255, BRIGHT_FULL);

          needsLedsUpdate = true;
          noteStates[pinCounter] = true;

          activeKeyForDisplay = pinCounter + 1;
          activeNoteForDisplay = midiNote;
          activeStatusForDisplay = (targetChannel == 2) ? "QT " : "ON ";
        }
      } 
      // 2. لحظة رفع الأصبع (جدولة إيقاف النغمة OFF بناءً على الصفحة 5)
      else if (rawTouchValue < releaseThreshold && noteStates[pinCounter] && !pendingNoteOff[pinCounter]) {
        pendingNoteOff[pinCounter] = true;
        noteReleaseTime[pinCounter] = millis() + (noteDurationVal * 10); 
      }

      // 3. تنفيذ إيقاف النغمة بعد انقضاء وقت التلاشي (Release)
      if (pendingNoteOff[pinCounter] && millis() >= noteReleaseTime[pinCounter]) {
        usbMIDI.sendNoteOff(midiNote, 0, targetChannel); 
        Serial1.write(statusByteOff); 
        Serial1.write(midiNote);
        Serial1.write(0);    

        setMainLineState(pinCounter, false); 
        
        uint8_t noteHue = map(originalNote, 52, 84, 0, 255);
        subLeds[ledIndex] = CHSV(noteHue, 255, BRIGHT_DIM);

        needsLedsUpdate = true;
        noteStates[pinCounter] = false;
        pendingNoteOff[pinCounter] = false;

        activeKeyForDisplay = pinCounter + 1;
        activeNoteForDisplay = midiNote;
        activeStatusForDisplay = "OFF";
      }

      pinCounter++; 
    }
  }
  
  if (needsLedsUpdate) {
    FastLED.show();
  }

  if ((activeKeyForDisplay != -1 || valuesChangedForDisplay) && (millis() - lastDisplayTime > displayInterval)) {
    updateOledDisplay(activeKeyForDisplay, activeNoteForDisplay, activeStatusForDisplay);
    lastDisplayTime = millis();
  }

  delay(2); 
}