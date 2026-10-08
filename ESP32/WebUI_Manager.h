#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "synth.h"

extern Synth synth;
extern int BASE_NOTE;

#ifdef ENABLE_GUI
#include "TextGUI.h"
extern TextGUI gui;
#endif

class WebUIManager {
private:
    WebServer server;

    // صفحة القانون المبسطة والخفيفة جداً لضمان سرعة التحميل
    const char* index_html = R"rawliteral(
<!DOCTYPE html>
<html lang="ar" dir="rtl">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>صفحة القانون الرقمي</title>
    <style>
        body { background-color: #121214; color: #e1e1e6; font-family: Tahoma, sans-serif; text-align: center; padding: 20px; }
        h1 { color: #00d26a; }
        .card { background: #202024; padding: 20px; margin: 15px auto; border-radius: 10px; max-width: 90%; width: 350px; border: 1px solid #2e2e34; box-shadow: 0 4px 6px rgba(0,0,0,0.3); }
        input[type=range] { width: 100%; margin-top: 15px; accent-color: #00d26a; }
        .val-label { font-size: 1.2rem; font-weight: bold; color: #00d26a; margin-top: 10px; display: block; }
    </style>
</head>
<body>
    <h1>🎛️ إعدادات القانون</h1>
    
    <div class="card">
        <label>مستوى الصوت العام (Volume)</label>
        <input type="range" id="vol" min="0" max="100" value="80" onchange="sendCmd('vol', this.value)">
        <span class="val-label" id="vol-val">80</span>
    </div>

    <div class="card">
        <label>صدى الصوت (Reverb)</label>
        <input type="range" id="rev" min="0" max="100" value="10" onchange="sendCmd('rev', this.value)">
        <span class="val-label" id="rev-val">10</span>
    </div>

    <div class="card">
        <label>تغيير الطبقة (Transpose)</label>
        <input type="range" id="tran" min="-24" max="24" value="0" onchange="sendCmd('tran', this.value)">
        <span class="val-label" id="tran-val">0</span>
    </div>

    <script>
        function sendCmd(type, val) {
            document.getElementById(type + '-val').innerText = val;
            fetch(`/set?type=${type}&val=${val}`)
            .then(response => console.log('تم الإرسال'))
            .catch(err => console.error('خطأ في الاتصال'));
        }
    </script>
</body>
</html>
)rawliteral";

public:
    WebUIManager() : server(80) {}

   void begin() {
        server.on("/", HTTP_GET, [this]() { 
            Serial.println("\n🌐 [WebUI] >>> تم الاتصال! المتصفح يطلب صفحة القانون <<<");
            server.send(200, "text/html", this->index_html); 
        });

        server.on("/set", HTTP_GET, [this]() { 
            if (server.hasArg("type") && server.hasArg("val")) {
                String type = server.arg("type");
                int val = server.arg("val").toInt();
                Serial.printf("🎚️ [WebUI] تم استلام أمر: %s = %d\n", type.c_str(), val);

                if (type == "vol") {
                    float fval = (float)val / 100.0f;
                    for (int ch = 0; ch < 16; ch++) synth.channels[ch].volume = fval;
                }
                else if (type == "rev") {
                    float fval = (float)val / 100.0f;
                    for (int ch = 0; ch < 16; ch++) synth.channels[ch].reverbSend = fval;
                }
                else if (type == "tran") {
                    BASE_NOTE = 60 + val;
                }

                #ifdef ENABLE_GUI
                gui.fullUpdate();
                #endif
            }
            server.send(200, "text/plain", "OK"); 
        });

        server.begin();
        Serial.println("✅ [WebUI] خادم الويب يعمل! بانتظار اتصال المتصفح...");
    }

        

    void process() {
        server.handleClient();
    }
}; // <--- هذا هو القوس الذي كان مفقوداً وسبب كل المشاكل!

extern WebUIManager webui;