# MIDI Qanun Project

A next-generation, desktop-sized electronic zither that bridges the rich musical heritage of the Middle Eastern Qanun and the Asian Gayageum with modern MIDI and digital synthesis. 

This project moves away from the traditional 4x4 pad matrix, introducing a linear 16-string tactile interface designed for organic musical expression, polyphonic velocity, and true microtonal performance.

## 🌟 Concept & Philosophy

The MIDI Qanun is designed for modern bedroom producers and musicians who seek the physical, emotional connection of a traditional acoustic instrument in a compact desktop format. By aligning 16 touch-sensitive horizontal "strings," the instrument preserves thousands of years of human muscle memory. It naturally supports striking techniques (like the Santur) and fluid sweeping motions (Glissando), offering a deeply human feel that standard square pads cannot replicate.

## ✨ Key Features

*   **True Polyphonic Expression:** A custom-engineered sensor stack combining an **ITO (Indium Tin Oxide)** transparent sheet, **Velostat**, and interdigital PCB copper pads. This allows for independent pressure and velocity reading on every single string simultaneously.
*   **Per-Note Microtuning:** Features 16 dedicated analog knobs (multiplexed via CD4067BE) and 7 quick-access quarter-tone toggle buttons. Unlike standard pitch-bend that affects the entire channel, this uses a custom software matrix to detune specific notes by -50 cents without disrupting polyphonic chords.
*   **Interactive Visualizer:** Reverse-mounted **SK6812MINI-E-012** addressable LEDs illuminate each string from beneath the PCB, shining through the Velostat and ITO layers to provide zero-profile, dynamic visual feedback based on touch velocity.
*   **Standalone Synthesis:** Powered by an **ESP32-S3** (utilizing 8MB PSRAM), the instrument processes its own audio natively using an SF2 SoundFont engine, eliminating the strict need for external computers.
*   **Organic Modulation:** An integrated FSR (Force Sensing Resistor) strip allows for natural, pressure-based vibrato and pitch bending, mimicking the technique of pressing a physical string behind the bridge.

## 🛠️ Hardware Stack

The hardware is designed for modularity and ease of maintenance, with a focus on seamless I2C integration and efficient GPIO routing.

*   **Microcontroller:** ESP32-S3 (Socketed via machine pin headers for easy swapping/upgrades).
*   **Touch Controller:** Trill Craft capacitive sensor (I2C Address: `0x28`).
*   **Multiplexing:** CD4067BE 16-channel analog multiplexer for the tuning knobs and Velostat pressure pads.
*   **Display:** 128x64 OLED Screen (I2C Address: `0x3D`) + Rotary Encoder for UI navigation.
*   **Top Surface:** A custom graphic overlay sitting on top of the continuous ITO sheet for a sleek, professional finish.
*   **PCB Design:** Custom layout designed in KiCad, optimized for JLCPCB SMT assembly.

## 📁 Repository Structure

```text
Qanun-project/
├── Hardware/               # KiCad project files, schematics, and PCB layouts
│   ├── Gerber/             # Exported manufacturing files for JLCPCB
│   └── 3D_Models/          # STEP files for the enclosure and custom panels
├── Firmware/               # C++/Arduino source code for the ESP32-S3
│   ├── src/                # Main logic, synth engine, and Trill integration
│   └── include/            # Header files and configuration matrices
├── Assets/                 # SoundFont (SF2) files and UI graphic overlay designs (PDF/DXF)
└── README.md
