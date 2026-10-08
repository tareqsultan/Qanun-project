#ifndef AI_DELAY_H
#define AI_DELAY_H

#include <Arduino.h>

#define AI_BUFFER_SIZE 64 

extern int BASE_NOTE;
extern const int majorScale[7];

class AIDelay {
public:
    bool enabled = false;           
    bool fade_out_enabled = true;   
    
    uint16_t bpm = 120;             
    uint8_t  division = 0;          
    
    uint8_t  max_feedback = 3;      
    // 💡 المستويات: 0=عادي, 1=هارموني, 2=عشوائي منتظم, 3=سلاسل ماركوف
    uint8_t  rebellion_level = 3;   
    
private:
    uint8_t note_buffer[AI_BUFFER_SIZE];
    uint32_t time_buffer[AI_BUFFER_SIZE]; 
    
    uint8_t notes_recorded = 0;
    uint8_t buffer_index = 0;
    
    uint32_t last_played_time = 0;
    uint32_t last_recorded_time = 0; 
    bool ready_to_reply = false;
    
    bool is_playing_reply = false;
    uint8_t play_index = 0;
    uint32_t last_reply_time = 0;
    uint8_t current_feedback_loop = 0; 

    // 💡 متغير لتتبع النغمة السابقة في سلسلة ماركوف
    uint8_t markov_last_note = 0;

    uint32_t getDelayTime() {
        uint32_t quarter_note_ms = 60000 / bpm;
        if (division == 1) return quarter_note_ms / 2;
        if (division == 2) return quarter_note_ms / 4;
        return quarter_note_ms;
    }

    uint8_t quantizeToScale(int note) {
        int relative_note = note - BASE_NOTE;
        int octave = relative_note >= 0 ? relative_note / 12 : (relative_note - 11) / 12;
        int semitone = relative_note - (octave * 12);
        if (semitone < 0) semitone += 12;

        int closest_note = majorScale[0];
        int min_diff = 12;
        
        for (int i = 0; i < 7; i++) {
            int diff = abs(semitone - majorScale[i]);
            if (diff < min_diff) {
                min_diff = diff;
                closest_note = majorScale[i];
            }
        }
        
        int snapped = BASE_NOTE + (octave * 12) + closest_note;
        if (snapped < 0) snapped = 0;
        if (snapped > 127) snapped = 127;
        return (uint8_t)snapped;
    }

    // 💡 دالة محرك ماركوف
    uint8_t getNextMarkovNote(uint8_t current_note) {
        if (notes_recorded < 2) return current_note; // لا يوجد بيانات كافية

        uint8_t possible_next_notes[AI_BUFFER_SIZE];
        uint8_t count = 0;

        // 1. البحث في العزف المسجل عن كل مرة ظهرت فيها هذه النغمة، وتسجيل النغمة التي تلتها
        for (uint8_t i = 0; i < notes_recorded - 1; i++) {
            if (note_buffer[i] == current_note) {
                possible_next_notes[count] = note_buffer[i + 1];
                count++;
            }
        }

        // 2. إذا وجدنا مسارات محتملة، نختار واحداً منها عشوائياً (هذا يحقق الاحتمالية الرياضية)
        if (count > 0) {
            uint8_t random_index = random(0, count);
            return possible_next_notes[random_index];
        } else {
            // 3. طريق مسدود (النغمة كانت آخر نغمة عزفتها). نختار نغمة عشوائية من عزفك لنبدأ سلسلة جديدة
            return note_buffer[random(0, notes_recorded)];
        }
    }

public:
    AIDelay() {}

    void recordNote(uint8_t note) {
        if (!enabled) return; 
        
        uint32_t current_time = millis();
        uint32_t time_delta = 0;
        uint32_t current_delay_ms = getDelayTime(); 
        
        if (notes_recorded > 0) {
            time_delta = current_time - last_recorded_time;
            
            // 💡 حماية من التشوه: منع الذكاء الاصطناعي من دمج النغمات السريعة جداً في نفس اللحظة
            if (time_delta < 20) time_delta = 20; 
            
            if (time_delta > 2000) {
                time_delta = current_delay_ms;
            }
        } else {
            time_delta = current_delay_ms; 
        }
        
        note_buffer[buffer_index] = note;
        time_buffer[buffer_index] = time_delta; 
        
        buffer_index = (buffer_index + 1) % AI_BUFFER_SIZE;
        if (notes_recorded < AI_BUFFER_SIZE) notes_recorded++;
        
        last_recorded_time = current_time;
        last_played_time = current_time;
        ready_to_reply = true;
    }

    bool process(uint8_t &out_note, uint8_t &out_velocity) {
        if (!enabled || notes_recorded == 0) return false;

        uint32_t current_delay_ms = getDelayTime();

        if (ready_to_reply && !is_playing_reply && (millis() - last_played_time >= current_delay_ms)) {
            is_playing_reply = true; 
            play_index = 0;
            current_feedback_loop = 0; 
            ready_to_reply = false;
            
            // 💡 تحديث: ضبط نقطة البداية لماركوف لتكون أول نغمة عزفتها
            markov_last_note = note_buffer[0]; 
            
            last_reply_time = millis() - time_buffer[0]; 
        }

        if (is_playing_reply) {
            uint32_t current_target_delay = time_buffer[play_index];
            
            if (millis() - last_reply_time >= current_target_delay) {
                // 💡 التعديل للحفاظ على ثبات الإيقاع (الشبكة الزمنية)
                last_reply_time += current_target_delay; 
                
                if (play_index < notes_recorded) {
                    uint8_t original_note = note_buffer[play_index];
                    play_index++;
                    
                    if (rebellion_level == 0) {
                        out_note = original_note; 
                    } 
                    else if (rebellion_level == 1) {
                        out_note = quantizeToScale(original_note + 7); 
                    } 
                    else if (rebellion_level == 2) {
                        int shift = random(-4, 5); 
                        out_note = quantizeToScale(original_note + shift); 
                    } 
                    // 💡 المستوى الجديد: سلاسل ماركوف الموسيقية
                    else if (rebellion_level == 3) {
                        uint8_t markov_note = getNextMarkovNote(markov_last_note);
                        out_note = quantizeToScale(markov_note); // تأكيد بقائها ضمن المقام
                        markov_last_note = markov_note; // تحديث النغمة للسلسلة القادمة
                    }
                    else {
                        out_note = original_note; 
                    }
                    
                    int calc_vel = 90; 
                    if (fade_out_enabled) {
                        calc_vel -= (current_feedback_loop * 25); 
                    }
                    out_velocity = (calc_vel < 10) ? 10 : calc_vel; 
                    
                    return true; 
                    
                } else {
                    current_feedback_loop++;
                    
                    if (current_feedback_loop < max_feedback) {
                        play_index = 0; 
                        // إعادة ضبط ماركوف عند بداية الـ Loop الجديد
                        markov_last_note = note_buffer[0]; 
                    } else {
                        is_playing_reply = false;
                        notes_recorded = 0;
                        buffer_index = 0;
                    }
                    return false;
                }
            }
        }
        return false;
    }
};

extern AIDelay ai_synth;

#endif