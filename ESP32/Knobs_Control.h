#pragma once
#include <Arduino.h>

class Knobs_Control {
private:
    int pin_S0 = 3;
    int pin_S1 = 2;
    int pin_S2 = 46;
    int pin_S3 = 47;
    int pin_SIG = 1;

    int last_cc_value[16];
    int raw_buffer[16] = {0}; 
    bool first_read = true;
    
    uint32_t last_scan_time = 0;

public:
    Knobs_Control() {
        for (int i = 0; i < 16; i++) {
            last_cc_value[i] = 0;
        }
    }

    void begin() {
        pinMode(pin_S0, OUTPUT);
        pinMode(pin_S1, OUTPUT);
        pinMode(pin_S2, OUTPUT);
        pinMode(pin_S3, OUTPUT);
        pinMode(pin_SIG, INPUT);
        
        analogReadResolution(12);
        Serial.println("[Knobs] CD4067 Ready (Idle - Ready for Groovebox/WebUI)");
    }

    void process() {
        // فحص كل 50 مللي ثانية فقط لتوفير موارد المعالج
        if (millis() - last_scan_time < 50) return;
        last_scan_time = millis(); 

        for (int i = 0; i < 16; i++) {
            // توجيه شريحة CD4067 للقناة المطلوبة
            digitalWrite(pin_S0, bitRead(i, 0));
            digitalWrite(pin_S1, bitRead(i, 1));
            digitalWrite(pin_S2, bitRead(i, 2));
            digitalWrite(pin_S3, bitRead(i, 3));
            
            delayMicroseconds(10); 
            int current_read = analogRead(pin_SIG);

            if (first_read) {
                raw_buffer[i] = current_read;
            } 

            // عازل لمنع تذبذب الإشارة (Anti-Jitter)
            if (abs(current_read - raw_buffer[i]) > 40) {
                raw_buffer[i] = current_read; 
                
                int cc_value = map(raw_buffer[i], 0, 4095, 0, 127);
                cc_value = constrain(cc_value, 0, 127);
                
                if (cc_value != last_cc_value[i]) {
                    last_cc_value[i] = cc_value;
                    // يتم تخزين القيم هنا في الذاكرة دون إرسال أوامر MIDI
                }
            }
        }
        
        if (first_read) first_read = false; 
    }

    // دوال لجلب القيمة المستقرة لأي مقبض عند استدعائه من الـ WebUI أو Groovebox
    int getValue(uint8_t index) const {
        if (index < 16) return last_cc_value[index];
        return 0;
    }

    int getRaw(uint8_t index) const {
        if (index < 16) return raw_buffer[index];
        return 0;
    }
};

extern Knobs_Control knobs_control;