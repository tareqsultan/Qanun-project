#ifndef BEHAVIORAL_SYSTEM_H
#define BEHAVIORAL_SYSTEM_H

#include <Arduino.h>

extern Synth synth; 

class BehavioralSystem {
private:
    // ذاكرة لتسجيل وقت آخر ضربة لكل وتر (لحجب القراءات الوهمية)
    uint32_t last_hit_time[24] = {0};
    const uint32_t debounce_time = 35; // 35 مللي ثانية (فترة حماية من التأتأة)
    
public:
    bool enabled = true; 

    BehavioralSystem() {}

    // ==============================================================
    // 1. الدالة الأساسية: مخصصة لعزف العازف (مع حماية من التأتأة)
    // ==============================================================
    void triggerNote(uint8_t pad_index, uint8_t note, uint8_t velocity) {
        uint32_t current_time = millis();
        
        // 💡 فلتر منع التأتأة (Debounce): 
        // إذا كانت اللمسة الجديدة حدثت في أقل من 35 مللي ثانية من اللمسة السابقة، تجاهلها.
        if (current_time - last_hit_time[pad_index] < debounce_time) {
            return; 
        }
        last_hit_time[pad_index] = current_time;

        if (enabled) {
            uint8_t fixed_channel = (pad_index % 15) + 1; 
            synth.noteOn(fixed_channel, note, velocity);
        } else {
            synth.noteOn(0, note, velocity);
        }
    }

    // ==============================================================
    // 2. دالة نغمات الذكاء الاصطناعي (AI Delay)
    // ==============================================================
    void triggerAINote(uint8_t note, uint8_t velocity) {
        if (enabled) {
            synth.noteOn(15, note, velocity); 
        } else {
            synth.noteOn(0, note, velocity);
        }
    }
    
    // ==============================================================
    // 3. دالة الإيقاف (عند رفع الإصبع)
    // ==============================================================
    void releaseNote(uint8_t pad_index, uint8_t note) {
        if (enabled) {
            uint8_t fixed_channel = (pad_index % 15) + 1;
            synth.noteOff(fixed_channel, note);
        } else {
            synth.noteOff(0, note);
        }
    }
};

extern BehavioralSystem behavior_system;

#endif