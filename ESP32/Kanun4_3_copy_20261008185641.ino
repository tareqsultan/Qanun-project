 
#include <WiFi.h>
#include "WebUI_Manager.h"
#include "synth.h"
#include <Arduino.h>
#include "config.h"
#include "esp_task_wdt.h"
#include <float.h>
#include "misc.h" 
#include "esp_log.h"
#include <FS.h>
#include "SD_MMC.h"
#include <sd_defines.h>
//#include <SD_MMC.h>
#include <LittleFS.h>
#include "driver/gpio.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
//#include <SD.h>
#include <SPI.h>
#include <MIDI.h>
#include "synth.h"
#include "SF2Parser.h"
#include "adsr.h"
#include "voice.h"
#include "SynthState.h"
//#include <BLEMIDI_Transport.h>
//#include <hardware/BLEMIDI_ESP32.h>
#include "ai_delay.h"
#include "BehavioralSystem.h" // 💡 الاستدعاء الصحيح لملف النظام السلوكي
#include "TCA_Control.h"


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


float quarterToneOffset[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
TCA_Control tca_control;
int playingNotes[24] = {0};
#include "Knobs_Control.h"
#include "WebUI_Manager.h"
WebUIManager webui;

Knobs_Control knobs_control;

// إنشاء الكائن الفعلي للنظام ليكون متاحاً لجميع الأكواد
BehavioralSystem behavior_system;

// البناء الفعلي للكائن في الذاكرة (مرة واحدة فقط)
AIDelay ai_synth;

// ===================== إعدادات حساس العفق (Pitch Bend) للمستقبل =====================
#define FSR_PIN 34 // (قم بتغييره لاحقاً للمنفذ الذي ستستخدمه)
uint8_t last_played_pad = 0; // ذاكرة لتتبع آخر وتر تم عزفه
bool is_fsr_connected = false; // اجعله false حالياً لمنع التشويش، وغيره إلى true مستقبلاً
// ====================================================================================

// إنشاء كائن البلوتوث باسم الجهاز الذي سيظهر للمستخدمين
//BLEMIDI_CREATE_INSTANCE("S3_Smart_Qanun", BLE_MIDI);

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
       // BLE_MIDI.read(); // السماح للبلوتوث باستقبال البيانات
        
        synth.updateScores();
        knobs_control.process();
        tca_control.process(); 
        webui.process();
    // ===================== محرك عزف اللمس (MPR121) =====================
    uint16_t currtouched1 = cap1.touched();
    for (uint8_t i = 0; i < 12; i++) {
        int padIndex = i;
        int baseNote = BASE_NOTE + (padIndex / 7) * 12 + majorScale[padIndex % 7];
        
        if ((currtouched1 & _BV(i)) && !(lasttouched1 & _BV(i))) {
            last_played_pad = padIndex;
            
            // تطبيق الرفع أو الخفض الرقمي
            int modifiedNote = baseNote;
            int row = i % 7;
            if (row != -1) {
                int modifier = tca_control.getModifier(row);
                if (modifier == 0) modifiedNote += 1;      // بيمول
                else if (modifier == 2) modifiedNote -= 1; // دييز
            }
            playingNotes[padIndex] = modifiedNote; 
            
            uint8_t active_ch = (padIndex % 15) + 1;
            behavior_system.triggerNote(padIndex, modifiedNote, 100); 
            MIDI.sendNoteOn(modifiedNote, 100, active_ch); 
          //  BLE_MIDI.sendNoteOn(modifiedNote, 100, active_ch); 
            ai_synth.recordNote(modifiedNote);
        }
        if (!(currtouched1 & _BV(i)) && (lasttouched1 & _BV(i))) {
            int noteToStop = playingNotes[padIndex]; 
            uint8_t active_ch = (padIndex % 15) + 1;
            behavior_system.releaseNote(padIndex, noteToStop); 
            MIDI.sendNoteOff(noteToStop, 0, active_ch); 
          //  BLE_MIDI.sendNoteOff(noteToStop, 0, active_ch); 
        }
    }
    lasttouched1 = currtouched1;

    uint16_t currtouched2 = cap2.touched();
    for (uint8_t i = 0; i < 12; i++) { 
        int padIndex = i + 12; 
        int baseNote = BASE_NOTE + (padIndex / 7) * 12 + majorScale[padIndex % 7];

        if ((currtouched2 & _BV(i)) && !(lasttouched2 & _BV(i))) {
            last_played_pad = padIndex;
            
            int modifiedNote = baseNote;
            int row = i % 7;
            if (row != -1) {
                int modifier = tca_control.getModifier(row);
                if (modifier == 0) modifiedNote -= 1;
                else if (modifier == 2) modifiedNote += 1;
            }
            playingNotes[padIndex] = modifiedNote;
            
            uint8_t active_ch = (padIndex % 15) + 1;
            behavior_system.triggerNote(padIndex, modifiedNote, 100); 
            MIDI.sendNoteOn(modifiedNote, 100, active_ch); 
           // BLE_MIDI.sendNoteOn(modifiedNote, 100, active_ch); 
            ai_synth.recordNote(modifiedNote);
        }
        if (!(currtouched2 & _BV(i)) && (lasttouched2 & _BV(i))) {
            int noteToStop = playingNotes[padIndex];
            uint8_t active_ch = (padIndex % 15) + 1;
            behavior_system.releaseNote(padIndex, noteToStop); 
            MIDI.sendNoteOff(noteToStop, 0, active_ch); 
           // BLE_MIDI.sendNoteOff(noteToStop, 0, active_ch); 
        }
    }
    lasttouched2 = currtouched2;
// ===================================================================

        // ===================== معالجة الـ AI Delay =====================
        uint8_t ai_note = 0;
        uint8_t ai_velocity = 0;
        
        if (ai_synth.process(ai_note, ai_velocity)) {
            // توجيه نغمة الـ AI لتعزف عبر النظام السلوكي الذكي بدلاً من العزف المباشر
            behavior_system.triggerAINote(ai_note, ai_velocity);
            
            // سحب القناة النشطة للـ AI لإرسالها لأجهزة الـ MIDI الخارجية
           uint8_t ai_ch = 15;
            MIDI.sendNoteOn(ai_note, ai_velocity, ai_ch);
           // BLE_MIDI.sendNoteOn(ai_note, ai_velocity, ai_ch);
        }
        // ==============================================================

#ifdef ENABLE_RGB_LED
        updateLed();
#endif
        vTaskDelay(1);
        taskYIELD();

#ifdef ENABLE_GUI
        if (__builtin_expect((gui_blocker == 0), 1)) {
            
            gui.encA = digitalRead(ENC0_A_PIN);
            gui.encB = digitalRead(ENC0_B_PIN);
            gui.btnState = digitalRead(BTN0_PIN);
            
            /* 
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
                           // Serial.printf("Quarter tone ON for note %d (Shifted by %d)\n", noteIndex, shift);
                        } else {
                            quarterToneOffset[noteIndex] = 0.0f; 
                          //  Serial.printf("Quarter tone OFF for note %d\n", noteIndex);
                        }
                    }
                    lastButtonState[i] = currentState;
                } 
                else if (i == 7) {
                    gui.btnState = currentState; 
                }
            }
            */
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
            //ESP_LOGI(TAG, "Avg cycles over %u frames: render = %u, write = %u", frame_count, avg_render, avg_write);
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
    // 💡 تفعيل إرسال سجلات ESP_LOGI إلى السيريال مونيتور عبر الـ USB
    Serial.setDebugOutput(true); 
    
    // 💡 تأخير زمني لضمان فتح السيريال مونيتور قبل بدء طباعة الرسائل
    delay(3000); 
    Serial.println("\n\n=== SETUP STARTED ===");

    // 1. فحص الذاكرة أولاً
    if (!psramFound()) {
        Serial.println("ERROR: PSRAM not found!");
        ESP_LOGE(TAG, "PSRAM not found!");
        while(true);
    }
    Serial.println("SUCCESS: PSRAM Found!");

    // 2. تهيئة منفذ USB والميدي فوراً لمنع الكمبيوتر من عمل Reset للوحة
#if MIDI_IN_DEV == USE_USB_MIDI_DEVICE
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

    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.setHandlePitchBend(handlePitchBend);
    MIDI.setHandleControlChange(handleControlChange);   
    MIDI.setHandleProgramChange(handleProgramChange);
    MIDI.setHandleSystemExclusive(handleSystemExclusive);
    
    delay(800); 
    Serial.println("SUCCESS: MIDI started");
    ESP_LOGI(TAG, "MIDI started");

    // 3. الاتصال بالشبكة وطباعة الـ IP
    Serial.println("Connecting to WiFi: poco...");
    ESP_LOGI("WiFi", "Connecting to poco...");
    WiFi.mode(WIFI_STA);
    WiFi.begin("poco", "90926297");

    // 💡 التعديل الأهم: محاولة الاتصال لمدة 10 ثوانٍ فقط، ثم التجاوز لتجنب التجميد
    int wifi_retries = 0;
    while (WiFi.status() != WL_CONNECTED && wifi_retries < 20) {
        delay(500); 
        Serial.print(".");
        wifi_retries++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nSUCCESS: WiFi Connected!");
        WiFi.setSleep(false); // 💡 يمنع انقطاع الحزم وانهيار الاتصال
        Serial.print("🌐 افتح هذا الرابط في المتصفح: http://");
        Serial.println(WiFi.localIP());
        Serial.println("===============================");
        ESP_LOGI("WiFi", "Connected!");
        ESP_LOGI("WiFi", "Open this IP in browser: http://%s", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("\nWARNING: WiFi Failed! Continuing offline...");
    }

    // 4. تشغيل الواجهة
    webui.begin();

#if MIDI_IN_DEV == USE_USB_MIDI_DEVICE
    // Change USB Device Descriptor Parameter
    // USB.VID(0x1209);
    // USB.PID(0x1304);
    // USB.productName("S3 SF2 Synth");
    // USB.manufacturerName("copych");
    // USB.usbVersion(0x0200);
    // USB.usbClass(TUSB_CLASS_AUDIO);
    // USB.usbSubClass(0x00);
    // USB.usbProtocol(0x00);
    // USB.usbAttributes(0x80);
#endif
    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.setHandlePitchBend(handlePitchBend);
    MIDI.setHandleControlChange(handleControlChange);   
    MIDI.setHandleProgramChange(handleProgramChange);
    MIDI.setHandleSystemExclusive(handleSystemExclusive);
    
    delay(800);

    // ---------------------------------------------------------
    // تهيئة قارئ الذاكرة OPEN-SMART باستخدام SDMMC (4-bit)
    // ---------------------------------------------------------
    SD_MMC.setPins(12, 11, 13, 14, 9, 10); 
    
    pinMode(11, INPUT_PULLUP); // CMD
    pinMode(13, INPUT_PULLUP); // D0
    pinMode(14, INPUT_PULLUP); // D1
    pinMode(9,  INPUT_PULLUP); // D2
    pinMode(10, INPUT_PULLUP); // D3 (CS)

    if (!SD_MMC.begin("/sdcard", true, false, 20000)) { 
        ESP_LOGE(TAG, "SDMMC init failed! Check wiring and pull-ups.");
        Serial.println("ERROR: SDMMC init failed!");
    } else {
        ESP_LOGI(TAG, "SDMMC initialized successfully");
        Serial.println("SUCCESS: SDMMC initialized successfully!");
    }

#ifdef ENABLE_GUI
    gui.begin();
    
    if (WiFi.status() == WL_CONNECTED) {
        // طباعة الـ IP مباشرة بدون كلمات إضافية لضمان ظهوره
        String ipMsg = WiFi.localIP().toString();
        gui.busyMessage(ipMsg.c_str());
        delay(4000);
    } else {
        gui.busyMessage("Synth Loading...");
    }
    
    ESP_LOGI(TAG, "GUI splash");
#endif

    // --- تهيئة حساسات اللمس ---
    Wire.end();
    Wire.begin(4, 5);
    Wire.setClock(100000); 
    tca_control.begin();
    ESP_LOGI(TAG, "I2C Bus started at 100kHz on pins 4 and 5");
    knobs_control.begin();
    if (!cap1.begin(0x5A, &Wire)) { 
        ESP_LOGE(TAG, "MPR121 #1 not found");
    }
    if (!cap2.begin(0x5B, &Wire)) { 
        ESP_LOGE(TAG, "MPR121 #2 not found");
    }

#ifdef ENABLE_REVERB
    reverb.init();
    ESP_LOGI(TAG, "Reverb FX started");
#endif

#ifdef ENABLE_DELAY
    delayfx.init();
    ESP_LOGI(TAG, "Delay FX started");
#endif

    synth.begin();
    delay(500);
    ESP_LOGI(TAG, "Synth is starting");

    for(int i = 1; i < 16; i++) {
        synth.channels[i].program = synth.channels[0].program;
        synth.channels[i].bankMSB = synth.channels[0].bankMSB;
        synth.channels[i].bankLSB = synth.channels[0].bankLSB;
        synth.channels[i].volume = synth.channels[0].volume;
        synth.channels[i].pan = synth.channels[0].pan;
        synth.channels[i].reverbSend = synth.channels[0].reverbSend;
        synth.controlChange(i, 7, (uint8_t)(synth.channels[0].volume * 127));
        synth.controlChange(i, 10, (uint8_t)(synth.channels[0].pan * 127));
    }

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
    xTaskCreatePinnedToCore( control_task, "ControlTask", 8192, NULL, 8, &Task2, 1 );

#ifdef ENABLE_GUI
    xTaskCreatePinnedToCore( gui_task, "GUITask", 5000, NULL, 5, &Task3, 1 );
#endif

    vTaskDelay(30);
    pinMode(6, INPUT_PULLUP);
    
    Serial.println("=== SETUP COMPLETE ===");
    ESP_LOGI(TAG, "SF2 Synth ready");
}


// ====================== LOOP ================= KILL IT OR NOT =========================================================
void loop() {
    //vTaskDelete(NULL);
  webui.process();
    delay(10);
}