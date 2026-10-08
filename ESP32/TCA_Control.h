#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCA8418.h>
#include "BehavioralSystem.h"

extern float quarterToneOffset[12];
extern Synth synth; 
extern int BASE_NOTE; // قراءة الترانسبوز العام

// الأبعاد الطبيعية للدرجات السبع بالنسبة للنغمة الأساسية (C=0, D=2, E=4, F=5, G=7, A=9, B=11)
const int rowToNote[7] = {0, 2, 4, 5, 7, 9, 11};

// ==============================================================
// 1. نظام التحكم بالمقامات عبر لوحة TCA8418
// ==============================================================
class TCA_Control {
private:
    Adafruit_TCA8418 keypad;
    
    // حالة العُرب لكل وسادة من الوسائد السبع (0 إلى 6):
    // -1 = طبيعي (Natural)
    //  0 = رفع نصف درجة رقمياً (Sharp)
    //  1 = ربع تون عبر SysEx (Quarter Flat)
    //  2 = خفض نصف درجة رقمياً (Flat)
    int activeModifier[7] = {-1, -1, -1, -1, -1, -1, -1};
    int lastBaseNote = -999;

public:
    void begin() {
        if (!keypad.begin(0x34, &Wire)) { 
            Serial.println("[TCA8418] Not found"); 
        } else {
            keypad.matrix(7, 3); 
            keypad.flush(); 
            Serial.println("[TCA8418] Keypad Ready & Initialized!");
        }
    }

    int getModifier(int row) {
        if (row < 0 || row > 6) return -1;
        return activeModifier[row];
    }

    // 💡 دالة إرسال رسالة SysEx لضبط السلم الموسيقي (Scale Tuning)
    // أصبحت عامة (Public) وتأخذ بعين الاعتبار قيمة BASE_NOTE الحالية
    void updateQuarterToneSysEx() {
        uint8_t sysexData[22] = {
            0xF0, 0x41, 0x10, 0x42, 0x12, 0x40, 0x11, 0x40, 
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 
            0x00, 0xF7
        };

        // تصفير مصفوفة الربع تون الداخلية
        for (int n = 0; n < 12; n++) {
            quarterToneOffset[n] = 0.0f;
        }

        int transposeOffset = BASE_NOTE - 60;

        for (int r = 0; r < 7; r++) {
            // حساب النغمة الموسيقية الفعلية الناتجة عن هذا الصف الفيزيائي بعد الترانسبوز
            int actualNoteClass = ((rowToNote[r] + transposeOffset) % 12 + 12) % 12;

            if (activeModifier[r] == 1) { // زر c1 (ربع تون مفعل لهذا الوتر)
                sysexData[8 + actualNoteClass] = 64 - 50;    // خفض 50 سنت
                quarterToneOffset[actualNoteClass] = -50.0f; // تحديث مصفوفة محرك الصوت
            }
        }

        // حساب Checksum لمعيار Roland GS
        int sum = 0;
        for (int i = 5; i <= 19; i++) sum += sysexData[i];
        sysexData[20] = (128 - (sum % 128)) & 0x7F;

        // إرسال رسالة SysEx لمحرك الصوت
        synth.handleSysEx(sysexData, 22);
        Serial.println("[TCA8418] SysEx Scale Tuning Updated with Transpose!");
    }

    void process() {
        // إذا تغيّر الـ Transpose من القائمة، نحدّث ترددات الربع تون فوراً للدرجات الجديدة
        if (BASE_NOTE != lastBaseNote) {
            lastBaseNote = BASE_NOTE;
            updateQuarterToneSysEx();
        }

        if (keypad.available() > 0) {
            int k = keypad.getEvent();
            bool pressed = k & 0x80; 
            int keyNum = k & 0x7F;   
            
            int row = (keyNum - 1) / 10; // من 0 إلى 6 (الوسائد الفيزيائية 1 إلى 7)
            int col = (keyNum - 1) % 10; // من 0 إلى 2 (عرب الوتر)
            
            if (pressed && row >= 0 && row <= 6 && col >= 0 && col <= 2) {
                // تبديل حالة الزر (Toggle)
                if (activeModifier[row] == col) {
                    activeModifier[row] = -1; // العودة للنغمة الطبيعية
                } else {
                    activeModifier[row] = col; // تفعيل العربة
                }

                updateQuarterToneSysEx();
            }
        }
    }
};

// ==============================================================
// 2. نظام حساسات اللمس وتطبيق التحويلات الرقمية
// ==============================================================
class Touch_Control {
private:
    static const int NUM_PADS = 24; 
    bool wasTouched[NUM_PADS] = {false};
    int playingNote[NUM_PADS] = {0}; 
    
    // التردد الطبيعي لكل وسادة عند Transpose = 0 (C4 = 60)
    const int defaultPadNotes[NUM_PADS] = {
        60, 62, 64, 65, 67, 69, 71, // أوكتاف 1 (الوسائد 0 إلى 6)
        72, 74, 76, 77, 79, 81, 83, // أوكتاف 2 (الوسائد 7 إلى 13)
        84, 86, 88, 89, 91, 93, 95, // أوكتاف 3 (الوسائد 14 إلى 20)
        96, 98, 100                 // تتمة (الوسائد 21 إلى 23)
    };

public:
    Touch_Control() {}

    void begin() {
        Serial.println("[Touch_Control] Touch Sensors Ready!");
    }

    void process(TCA_Control& tca) {
        int transposeOffset = BASE_NOTE - 60;

        for (int i = 0; i < NUM_PADS; i++) {
            bool isTouched = false; // استبدلها بدالة قراءة الحساس لديك
            
            // 💡 1. حساب النغمة الأساسية متضمنة الـ Transpose
            int baseNote = defaultPadNotes[i] + transposeOffset;

            // 💡 2. الارتباط الفيزيائي المباشر: الوسادة i تتبع صف العرب (i % 7) دائماً وأبداً
            int row = i % 7;

            if (isTouched && !wasTouched[i]) {
                wasTouched[i] = true;
                
                int modifiedNote = baseNote;
                int modifier = tca.getModifier(row);

                // تطبيق الرفع والخفض النصف-توني على النغمة الناتجة
                if (modifier == 0) {
                    modifiedNote += 1; // c0: دييز
                } else if (modifier == 2) {
                    modifiedNote -= 1; // c2: بيمول
                }
                // c1: ربع التون مطبق مسبقاً عبر SysEx و quarterToneOffset

                playingNote[i] = modifiedNote;
                
                int velocity = 100;
                behavior_system.triggerNote(i, modifiedNote, velocity);
                
            } 
            else if (!isTouched && wasTouched[i]) {
                wasTouched[i] = false;
                behavior_system.releaseNote(i, playingNote[i]);
            }
        }
    }
};

extern TCA_Control tca_control;
extern Touch_Control touch_control;