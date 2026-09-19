/*
* ESP32-S3 SF2 Sampler
* An SF2 (SoundFont 2) based synthesizer designed specifically for the ESP32-S3 microcontroller.
* This project leverages the enhanced memory capabilities of the ESP32-S3 (with PSRAM)
* to efficiently load and play SoundFont samples, providing a compact and powerful sampler solution.
* The ESP32-S3 SF2 Sampler is a sampler firmware that runs exclusively on the ESP32-S3 variant
* due to its improved PSRAM and memory management compared to the original ESP32. 
* It supports external DACs like the PCM5102 for high-quality audio output and 
* uses the built-in USB hardware of the ESP32-S3 to function as a USB MIDI device.
* GM/GS/XG support is partlially implemented (i.e. with 2MBGMGS.sf2 bank).
*
* Libraries used:
* Arduino MIDI library https://github.com/FortySevenEffects/arduino_midi_library
* Optional. Using RGB LEDs requires FastLED library https://github.com/FastLED/FastLED
*
* (c) Copych 2025, License: MIT https://github.com/copych/SF2_Sampler?tab=MIT-1-ov-file#readme
* 
* More info:
* https://github.com/copych/SF2_Sampler
*/
// ===================== متغيرات أزرار الربع تون عبر CD4051 =====================
const int pinA = 6;  // يوصل بـ Pin 11 (A) في الشريحة
const int pinB = 7;  // يوصل بـ Pin 10 (B) في الشريحة
const int pinC = 8;  // يوصل بـ Pin 9 (C) في الشريحة
const int comPin = 1; // يوصل بـ Pin 3 (COM) في الشريحة

const int noteIndices[7] = {0, 2, 4, 5, 7, 9, 11}; 
float quarterToneOffset[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// مصفوفة الحالة السابقة (نحتاج 7 فقط للنغمات الأساسية السبع)
bool lastButtonState[7] = {true, true, true, true, true, true, true};
// ===========================================================================
// ===================== متغيرات حساسات اللمس MPR121 =====================
#include <Adafruit_MPR121.h>
Adafruit_MPR121 cap1 = Adafruit_MPR121();
Adafruit_MPR121 cap2 = Adafruit_MPR121();

uint16_t lasttouched1 = 0; 
uint16_t lasttouched2 = 0; 
int BASE_NOTE = 60;  // نغمة البداية (Middle C)

// مصفوفة أبعاد سلم C Major (الأنصاف الصوتية انطلاقاً من نغمة الأساس)
const int majorScale[7] = {0, 2, 4, 5, 7, 9, 11}; 
// =======================================================================
#pragma packed(16)
#pragma GCC optimize ("O3")
#pragma GCC optimize ("fast-math")
#pragma GCC optimize ("unsafe-math-optimizations")
#pragma GCC optimize ("no-math-errno")

static const char* TAG = "Main";

#define FORMAT_LITTLEFS_IF_FAILED

#include <Arduino.h>
#include "config.h"
#include "esp_task_wdt.h"
#include <float.h>
#include "misc.h" 
#include "esp_log.h"
#include <FS.h>

//#include "SD_MMC.h"
#include <LittleFS.h>

#include "driver/gpio.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
#include <SD.h>
#include <SPI.h>
#include <MIDI.h>
#include "synth.h"
#include "SF2Parser.h"
#include "adsr.h"
#include "voice.h"
#include "SynthState.h"

#ifdef ENABLE_RGB_LED
    #include "rgb_led.h"
#endif

#if MIDI_IN_DEV == USE_USB_MIDI_DEVICE
    #include "src/usbmidi/src/USB-MIDI.h"
#endif

#include "i2s_in_out.h" 

// tasks for Core0 and Core1
TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t Task3;

int Voice::usage; // counts voices internally


// ========================== MIDI Instance ===============================================================================================
#if MIDI_IN_DEV == USE_MIDI_STANDARD
    MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);
#endif

#if MIDI_IN_DEV == USE_USB_MIDI_DEVICE
    USBMIDI_CREATE_INSTANCE(0, MIDI); 
#endif

// ========================== Global devices ===============================================================================================
#ifdef ENABLE_CHORUS
    #include "fx_chorus.h"
    FxChorus    DRAM_ATTR   chorus;
#endif

#ifdef ENABLE_REVERB
    #include "fx_reverb.h"
    FxReverb    DRAM_ATTR   reverb;
#endif

#ifdef ENABLE_DELAY
    #include "fx_delay.h"
    FxDelay     DRAM_ATTR   delayfx;
#endif



I2S_Audio   AudioPort(I2S_Audio::MODE_OUT);
SF2Parser   parser(SF2_PATH);
Synth       synth(parser);

// ========================== Synth Settings ===================================================================================
SynthState state {
  synth.channels
#ifdef ENABLE_REVERB
  , reverb
#endif
#ifdef ENABLE_DELAY
  , delayfx
#endif
#ifdef ENABLE_CHORUS
  , chorus
#endif
};

// ========================== GUI ==============================================================================================
#ifdef ENABLE_GUI
    #include "TextGUI.h"
    TextGUI gui(synth, state);
#endif

// ========================== MIDI handlers ===============================================================================================
void handleNoteOn(byte ch, byte note, byte vel) {
#ifdef ENABLE_RGB_LED
    triggerLedFlash();
#endif
    //synth.printState();
    synth.noteOn(ch-1, note, vel);
}

void handleNoteOff(byte ch, byte note, byte vel) {
    synth.noteOff(ch-1, note);
}

void handlePitchBend(byte ch, int bend) {
    synth.pitchBend(ch-1, bend);
}

void handleControlChange(byte ch, byte control, byte value) {
    synth.controlChange(ch-1, control, value);
}

void handleProgramChange(uint8_t ch, uint8_t program) {
    ESP_LOGI("MIDI", "Program change on channel 0%u → program %u", ch-1, program);
    synth.programChange(ch-1, program);
}

void handleSystemExclusive( uint8_t* data, size_t len) {
    synth.handleSysEx( data,  len); 
}

#ifdef TASK_BENCHMARKING
// globals to be unsafely shared between tasks
    uint32_t DRAM_ATTR t0,t1,t2,t3,t4,t5,t6;
    uint32_t DRAM_ATTR dt1,dt2,dt3,dt4,dt5,dt6;
    uint32_t DRAM_ATTR total_render = 0;
    uint32_t DRAM_ATTR total_write  = 0;
#endif

    volatile uint32_t DRAM_ATTR frame_count  = 0;
    float DRAM_ATTR blockL[DMA_BUFFER_LEN];
    float DRAM_ATTR blockR[DMA_BUFFER_LEN];


// ========================== Core 0 Task 1 ===============================================================================================
// Core0 task -- AUDIO
static void IRAM_ATTR audio_task(void *userData) {
    vTaskDelay(20); 
    ESP_LOGI(TAG, "Starting Task1");



    while (true) {
        
#ifdef TASK_BENCHMARKING
        t0 = esp_cpu_get_cycle_count();
#endif


        synth.renderLRBlock(blockL, blockR);


#ifdef TASK_BENCHMARKING
        t1 = esp_cpu_get_cycle_count();
#endif


        AudioPort.writeBuffers(blockL, blockR);


#ifdef TASK_BENCHMARKING
        t2 = esp_cpu_get_cycle_count();

        total_render += (t1 - t0);
        total_write  += (t2 - t1);
#endif

        frame_count++;
    }
}

// ========================== Core 1 Task 2 ===============================================================================================
static void IRAM_ATTR control_task(void *userData) { 
    vTaskDelay(50);
    ESP_LOGI(TAG, "Starting Task2");
    
    while (true) { 
        MIDI.read();
        synth.updateScores();

        // ===================== محرك عزف اللمس (MPR121) =====================
        uint16_t currtouched1 = cap1.touched();
        for (uint8_t i = 0; i < 12; i++) {
            // حساب نغمة مقام العجم (C Major)
            int note = BASE_NOTE + (i / 7) * 12 + majorScale[i % 7];
            
            if ((currtouched1 & _BV(i)) && !(lasttouched1 & _BV(i))) {
                synth.noteOn(0, note, 100); 
                MIDI.sendNoteOn(note, 100, 1); 
            }
            if (!(currtouched1 & _BV(i)) && (lasttouched1 & _BV(i))) {
                synth.noteOff(0, note); 
                MIDI.sendNoteOff(note, 0, 1); 
            }
        }
        lasttouched1 = currtouched1;

        uint16_t currtouched2 = cap2.touched();
        for (uint8_t i = 0; i < 12; i++) { 
            int padIndex = i + 12;
            int note = BASE_NOTE + (padIndex / 7) * 12 + majorScale[padIndex % 7];

            if ((currtouched2 & _BV(i)) && !(lasttouched2 & _BV(i))) {
                synth.noteOn(0, note, 100); 
                MIDI.sendNoteOn(note, 100, 1); 
            }
            if (!(currtouched2 & _BV(i)) && (lasttouched2 & _BV(i))) {
                synth.noteOff(0, note); 
                MIDI.sendNoteOff(note, 0, 1); 
            }
        }
        lasttouched2 = currtouched2;
        // ===================================================================

#ifdef ENABLE_RGB_LED
        updateLed();
#endif
        vTaskDelay(1);
        taskYIELD();

#ifdef ENABLE_GUI
        if (__builtin_expect((gui_blocker == 0), 1)) {
            
            gui.encA = digitalRead(ENC0_A_PIN);
            gui.encB = digitalRead(ENC0_B_PIN);
            
            for (int i = 0; i < 8; i++) { 
                
                digitalWrite(pinA, bitRead(i, 0));
                digitalWrite(pinB, bitRead(i, 1));
                digitalWrite(pinC, bitRead(i, 2));
                
                delayMicroseconds(10); 
                
                bool currentState = digitalRead(comPin);
                
                if (i < 7) {
                    if (currentState == LOW && lastButtonState[i] == true) {
                        
                        // 💡 المعادلة الذكية: حساب مقدار الإزاحة بناءً على قيمة BASE_NOTE
                        int shift = (BASE_NOTE - 60) % 12;
                        // تطبيق الإزاحة على الزر ليتطابق دائماً مع الوسادة (مع ضمان عدم وجود رقم سالب)
                        int noteIndex = (noteIndices[i] + shift + 12) % 12;
                        
                        if (quarterToneOffset[noteIndex] == 0.0f) {
                            quarterToneOffset[noteIndex] = -50.0f; 
                            Serial.printf("Quarter tone ON for note %d (Shifted by %d)\n", noteIndex, shift);
                        } else {
                            quarterToneOffset[noteIndex] = 0.0f; 
                            Serial.printf("Quarter tone OFF for note %d\n", noteIndex);
                        }
                    }
                    lastButtonState[i] = currentState;
                } 
                else if (i == 7) {
                    gui.btnState = currentState; 
                }
            }

            gui.process();
            
        } else {
            gui_blocker--;
            if (gui_blocker < 0) { gui_blocker = 0; }
        }
#endif
        
        if (frame_count >= 64) {
#ifdef TASK_BENCHMARKING
            uint32_t avg_render = total_render / frame_count;
            uint32_t avg_write  = total_write  / frame_count;
            ESP_LOGI(TAG, "Avg cycles over %u frames: render = %u, write = %u", frame_count, avg_render, avg_write);
            total_render = 0;
            total_write  = 0;
#endif
            synth.updateActivity();
            frame_count  = 0;
        }
    }
}
        
    


#ifdef ENABLE_GUI
// ========================== Core 1 Task 3 ===============================================================================================
static void IRAM_ATTR gui_task(void *userData) { 
    vTaskDelay(50);
    ESP_LOGI(TAG, "Starting Task3");
    
    while (true) {
        if (gui_blocker == 0) {
            gui.draw();
        }
        taskYIELD();
    }
}
#endif

// ========================== SETUP ===============================================================================================
void setup() {

    Serial.begin(115200);
    delay(500); 
    
    if (!psramFound()) {
      ESP_LOGE(TAG, "PSRAM not found!");
      vTaskDelay(10);
      while(true);
    }
    
    //btStop(); 
    // تهيئة منافذ الأزرار وتفعيل المقاومة الداخلية
// تهيئة أطراف شريحة CD4051
pinMode(pinA, OUTPUT);
pinMode(pinB, OUTPUT);
pinMode(pinC, OUTPUT);
pinMode(comPin, INPUT_PULLUP);
#if MIDI_IN_DEV == USE_USB_MIDI_DEVICE
  // Change USB Device Descriptor Parameter
    USB.VID(0x1209);
    USB.PID(0x1304);
    USB.productName("S3 SF2 Synth");
    USB.manufacturerName("copych");
    USB.usbVersion(0x0200);
    USB.usbClass(TUSB_CLASS_AUDIO);
    USB.usbSubClass(0x00);
    USB.usbProtocol(0x00);
    USB.usbAttributes(0x80);
#endif

    // --- ADD THIS LINE TO ROUTE RX TO GPIO 42 ---
  //  Serial1.begin(31250, SERIAL_8N1, 42, -1);

    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.setHandlePitchBend(handlePitchBend);
    MIDI.setHandleControlChange(handleControlChange);   
    MIDI.setHandleProgramChange(handleProgramChange);
    MIDI.setHandleSystemExclusive(handleSystemExclusive);

    delay(800);
    ESP_LOGI(TAG, "MIDI started");

  // تهيئة ناقل SPI بالأطراف الفعلية (SCK=12, MISO=13, MOSI=11, CS=10)
    SPI.begin(12, 13, 11, 10);
    
    if (!SD.begin(10)) {
        ESP_LOGE(TAG, "SD init failed");
    } else {
        ESP_LOGI(TAG, "SD initialized via SPI");
    }
    if (!LittleFS.begin(true)) {
        ESP_LOGE(TAG, "LittleFS init failed");
    } else {
        ESP_LOGI(TAG, "LittleFS initialized");
    }

#ifdef ENABLE_GUI
    gui.begin();
    gui.busyMessage( "Synth Loading...");
    ESP_LOGI(TAG, "GUI splash");
#endif

    // --- إضافة تهيئة حساسات اللمس هنا ---
    if (!cap1.begin(0x5A)) {
        ESP_LOGE(TAG, "MPR121 #1 not found");
    } else {
        ESP_LOGI(TAG, "MPR121 #1 ready");
    }
    
    if (!cap2.begin(0x5B)) {
        ESP_LOGE(TAG, "MPR121 #2 not found");
    } else {
        ESP_LOGI(TAG, "MPR121 #2 ready");
    }
    // ------------------------------------

#ifdef ENABLE_REVERB
    reverb.init();
    ESP_LOGI(TAG, "Reverb FX started");
#endif

#ifdef ENABLE_DELAY
    delayfx.init();
    ESP_LOGI(TAG, "Delay FX started");
#endif
 

    synth.begin();
    ESP_LOGI(TAG, "Synth is starting");

#ifdef ENABLE_GUI
    gui.startMenu();
    ESP_LOGI(TAG, "GUI started");
#endif

    AudioPort.init(I2S_Audio::MODE_OUT);
    ESP_LOGI(TAG, "I2S Audio port started");

#ifdef ENABLE_RGB_LED
    setupLed();
    ESP_LOGI(TAG, "RGB LED started");
#endif


    xTaskCreatePinnedToCore( audio_task, "SynthTask", 5000, NULL, 8, &Task1, 0 );
    xTaskCreatePinnedToCore( control_task, "ControlTask", 5000, NULL, 8, &Task2, 1 );

#ifdef ENABLE_GUI
    xTaskCreatePinnedToCore( gui_task, "GUITask", 5000, NULL, 5, &Task3, 1 );
#endif

    vTaskDelay(30);

    ESP_LOGI(TAG, "SF2 Synth ready");
}


// ====================== LOOP ================= KILL IT OR NOT =========================================================
void loop() {
    vTaskDelete(NULL);
}
