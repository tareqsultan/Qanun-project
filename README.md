# MIDI Qanun Project

The **"Hybrid Son of the Asian Zither Family."** A next-generation, desktop-sized electronic instrument that bridges the rich musical heritage of the Middle Eastern Qanun and the Asian Gayageum with modern MIDI and digital synthesis. 

## 🌟 Concept & Philosophy

This instrument is not a literal digital replica of any single traditional acoustic instrument. Traditional flat zithers possess varying string counts depending on their cultural origin. Instead, this project deliberately features **16 linear strings and 16 dedicated tuning knobs**. 

This specific number is a direct homage to the digital heritage of modern music production—specifically the ubiquitous **4x4 pad matrix** and 16-step sequencers. By unfolding the standard digital 4x4 grid into a flat, 16-strip horizontal layout, the instrument speaks the native language of electronic desktop musicians. Yet, it physically responds to the ancient muscle memory, striking techniques (like the Santur), and fluid sweeping motions (Glissando) of the Middle Eastern and Asian flat zithers. 

It is the perfect hybrid: born in the modern digital desktop environment, but carrying the ancient, organic DNA of the Asian instrument family.

## ✨ Key Features

*   **True Polyphonic Expression:** A custom-engineered sensor stack combining an **ITO (Indium Tin Oxide)** transparent sheet, **Velostat**, and interdigital PCB copper pads. This allows for independent pressure and velocity reading on every single string simultaneously.
*   **Per-Note Microtuning:** Features 16 dedicated analog knobs (multiplexed via CD4067BE) and 7 quick-access quarter-tone toggle buttons. Unlike standard pitch-bend that affects the entire channel, this uses a custom software matrix to detune specific notes by -50 cents without disrupting polyphonic chords.
*   **Interactive Visualizer:** Reverse-mounted **SK6812MINI-E-012** addressable LEDs illuminate each string from beneath the PCB, shining through the Velostat and ITO layers to provide zero-profile, dynamic visual feedback based on touch velocity.
*   **Standalone Synthesis:** Powered by an **ESP32-S3** (utilizing 8MB PSRAM), the instrument processes its own audio natively using an SF2 SoundFont engine, eliminating the strict need for external computers.
*   **Organic Modulation:** An integrated FSR (Force Sensing Resistor) strip allows for natural, pressure-based vibrato and pitch bending, mimicking the technique of pressing a physical string behind the bridge.
*   **Future-Ready (AI Accompaniment):** The ESP32-S3’s embedded Wi-Fi lays the groundwork for real-time WebSocket connectivity, allowing future integrations with Cloud AI APIs for generative, scale-aware musical accompaniment.

## 🛠️ Hardware Evolution & Stack

Hardware design is a continuous evolution in this project. While the current primary development focuses on the cost-effective and powerful **ESP32-S3**, the repository proudly archives and supports earlier iterations built on the **Teensy** and **Daisy Seed** platforms, allowing makers to use the hardware they already have.

**Current Primary Setup:**
*   **Microcontroller:** ESP32-S3 (Socketed via machine pin headers for easy swapping/upgrades).
*   **Touch Controller:** Trill Craft capacitive sensor (I2C Address: `0x28`).
*   **Multiplexing:** CD4067BE 16-channel analog multiplexer for the tuning knobs and Velostat pressure pads.
*   **Display:** 128x64 OLED Screen (I2C Address: `0x3D`) + Rotary Encoder for UI navigation.
*   **Top Surface:** A custom graphic overlay sitting on top of the continuous ITO sheet for a sleek, professional finish.

## 📁 Repository Structure

# MIDI Qanun Project

The **"Hybrid Son of the Asian Zither Family."** A next-generation, desktop-sized electronic instrument that bridges the rich musical heritage of the Middle Eastern Qanun and the Asian Gayageum with modern MIDI and digital synthesis. 

## 🌟 Concept & Philosophy

This instrument is not a literal digital replica of any single traditional acoustic instrument. Traditional flat zithers possess varying string counts depending on their cultural origin. Instead, this project deliberately features **16 linear strings and 16 dedicated tuning knobs**. 

This specific number is a direct homage to the digital heritage of modern music production—specifically the ubiquitous **4x4 pad matrix** and 16-step sequencers. By unfolding the standard digital 4x4 grid into a flat, 16-strip horizontal layout, the instrument speaks the native language of electronic desktop musicians. Yet, it physically responds to the ancient muscle memory, striking techniques (like the Santur), and fluid sweeping motions (Glissando) of the Middle Eastern and Asian flat zithers. 

It is the perfect hybrid: born in the modern digital desktop environment, but carrying the ancient, organic DNA of the Asian instrument family.

## ✨ Key Features

*   **True Polyphonic Expression:** A custom-engineered sensor stack combining an **ITO (Indium Tin Oxide)** transparent sheet, **Velostat**, and interdigital PCB copper pads. This allows for independent pressure and velocity reading on every single string simultaneously.
*   **Per-Note Microtuning:** Features 16 dedicated analog knobs (multiplexed via CD4067BE) and 7 quick-access quarter-tone toggle buttons. Unlike standard pitch-bend that affects the entire channel, this uses a custom software matrix to detune specific notes by -50 cents without disrupting polyphonic chords.
*   **Interactive Visualizer:** Reverse-mounted **SK6812MINI-E-012** addressable LEDs illuminate each string from beneath the PCB, shining through the Velostat and ITO layers to provide zero-profile, dynamic visual feedback based on touch velocity.
*   **Standalone Synthesis:** Powered by an **ESP32-S3** (utilizing 8MB PSRAM), the instrument processes its own audio natively using an SF2 SoundFont engine, eliminating the strict need for external computers.
*   **Organic Modulation:** An integrated FSR (Force Sensing Resistor) strip allows for natural, pressure-based vibrato and pitch bending, mimicking the technique of pressing a physical string behind the bridge.
*   **Future-Ready (AI Accompaniment):** The ESP32-S3’s embedded Wi-Fi lays the groundwork for real-time WebSocket connectivity, allowing future integrations with Cloud AI APIs for generative, scale-aware musical accompaniment.

## 🛠️ Hardware Evolution & Stack

Hardware design is a continuous evolution in this project. While the current primary development focuses on the cost-effective and powerful **ESP32-S3**, the repository proudly archives and supports earlier iterations built on the **Teensy** and **Daisy Seed** platforms, allowing makers to use the hardware they already have.

**Current Primary Setup:**
*   **Microcontroller:** ESP32-S3 (Socketed via machine pin headers for easy swapping/upgrades).
*   **Touch Controller:** Trill Craft capacitive sensor (I2C Address: `0x28`).
*   **Multiplexing:** CD4067BE 16-channel analog multiplexer for the tuning knobs and Velostat pressure pads.
*   **Display:** 128x64 OLED Screen (I2C Address: `0x3D`) + Rotary Encoder for UI navigation.
*   **Top Surface:** A custom graphic overlay sitting on top of the continuous ITO sheet for a sleek, professional finish.

## 📁 Repository Structure

```text
Qanun-project/
├── Hardware/                 # KiCad project files, schematics, and PCB layouts
│   ├── Gerber/               # Exported manufacturing files for JLCPCB
│   └── 3D_Models/            # STEP files for the enclosure and custom panels
├── Firmware/                 # Source code organized by microcontroller
│   ├── ESP32-S3/             # (Active) Main logic, SF2 synth engine, and Trill I2C integration
│   ├── Teensy/               # (Archive/Alt) Firmware for Teensy 3.2 / 4.1 architectures
│   └── Daisy_Seed/           # (Archive/Alt) Firmware for the Daisy Seed audio platform
├── Assets/                   # SoundFont (SF2) files and UI graphic overlay designs
└── README.md

Qanun-project/
├── Hardware/                 # KiCad project files, schematics, and PCB layouts
│   ├── Gerber/               # Exported manufacturing files for JLCPCB
│   └── 3D_Models/            # STEP files for the enclosure and custom panels
├── Firmware/                 # Source code organized by microcontroller
│   ├── ESP32-S3/             # (Active) Main logic, SF2 synth engine, and Trill I2C integration
│   ├── Teensy/               # (Archive/Alt) Firmware for Teensy 3.2 / 4.1 architectures
│   └── Daisy_Seed/           # (Archive/Alt) Firmware for the Daisy Seed audio platform
├── Assets/                   # SoundFont (SF2) files and UI graphic overlay designs
└── README.md
*   **Multiplexing:** CD4067BE 16-channel analog multiplexer for the tuning knobs and Velostat pressure pads.
*   **Display:** 128x64 OLED Screen (I2C Address: `0x3D`) + Rotary Encoder for UI navigation.
*   **Top Surface:** A custom graphic overlay sitting on top of the continuous ITO sheet for a sleek, professional finish.

## 📁 Repository Structure

```text
Qanun-project/
├── Hardware/                 # KiCad project files, schematics, and PCB layouts
│   ├── Gerber/               # Exported manufacturing files for JLCPCB
│   └── 3D_Models/            # STEP files for the enclosure and custom panels
├── Firmware/                 # Source code organized by microcontroller
│   ├── ESP32-S3/             # (Active) Main logic, SF2 synth engine, and Trill I2C integration
│   ├── Teensy/               # (Archive/Alt) Firmware for Teensy 3.2 / 4.1 architectures
│   └── Daisy_Seed/           # (Archive/Alt) Firmware for the Daisy Seed audio platform
├── Assets/                   # SoundFont (SF2) files and UI graphic overlay designs (PDF/DXF)
└── README.md

Qanun-project/
├── Hardware/               # KiCad project files, schematics, and PCB layouts
│   ├── Gerber/             # Exported manufacturing files for JLCPCB
│   └── 3D_Models/          # STEP files for the enclosure and custom panels
├── Firmware/               # C++/Arduino source code for the ESP32-S3
│   ├── src/                # Main logic, synth engine, and Trill integration
│   └── include/            # Header files and configuration matrices
├── Assets/                 # SoundFont (SF2) files and UI graphic overlay designs (PDF/DXF)
└── README.md
