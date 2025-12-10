# 📟 Arduino Pomodoro Timer

A standalone hardware Pomodoro timer built for **CSCI 1600 – Real-Time & Embedded Software**.
This project uses an Arduino, TFT touchscreen, RGB LED, hardware interrupts, watchdog monitoring, onboard buzzer music, and optional WiFi logging to a backend server.

This folder contains **all microcontroller code** that runs the Pomodoro device.

---

## 🚀 Features

### ✔️ Full Pomodoro Cycle Automation

- **Focus → Break → Focus → ... → Long Break**
- Configurable durations (25/5/15 by default)
- Supports **fast testing mode**

### ✔️ TFT UI

- Home screen, active timer screen, pause/resume UI
- Touchscreen buttons for **Start / Pause / Resume / Reset**
- Clean visual states for each phase

### ✔️ RGB LED Status Indicator

- Red → Focus
- Green → Short Break
- Blue → Long Break
- White → Idle or Paused

### ✔️ Buzzer Alerts (RTTTL Music)

- Custom tones at the end of each session
- RTTTL parser included (`rtttl_parser.h/.ino`)

### ✔️ Hardware Interrupt Buttons

- START and RESET mapped to external interrupts
- Debouncing and ISR flag synchronization

### ✔️ Watchdog Timer

- Detects timing drift
- If a phase ends too early/late → **auto system reset**

### ✔️ Optional WiFi Logging

If connected to WiFi, the Arduino automatically sends session data to a backend server:

- POST `/session` for focus/break completion
- GET `/config` to fetch duration overrides

If WiFi fails, the device continues offline.

---

## 📁 File Overview

### **Arduino-Pomodoro-Timer.ino**

Main program file. Responsible for:

- Initialization (TFT, touchscreen, WiFi, buttons, LEDs)
- Pomodoro timer logic
- Phase transitions and UI updates
- Interrupt handling
- Watchdog monitoring
- WiFi POST/GET to external server
- Playing buzzer melodies

This is the primary file uploaded to the Arduino.

---

### **rtttl_parser.h**

Header file defining musical note constants and the function prototype for the RTTTL parser.

Contains:

- Frequency definitions for musical notes (B0 → DS8)
- `allNotes[]` lookup table
- Declaration of `rtttlToBuffers()`

Used to convert RTTTL strings into playable buzzer instructions.

---

### **rtttl_parser.ino**

Implementation of the RTTTL parser.

Provides:

- String cleaning and validation
- Parsing RTTTL headers (`b=`, `o=`, `d=`)
- Parsing each note, octave, sharp, dotted notes, rests
- Populating arrays of **frequencies** and **durations**
- Returns the total number of notes in a song

This enables the Arduino to play custom melodies using the buzzer.

---

## 🔌 Hardware Requirements

- **Arduino Uno R4 WiFi** (or compatible board)
- **2.8" ILI9341 TFT Display**
- **Resistive Touchscreen Overlay**
- **RGB LED** (3 PWM pins)
- **Momentary pushbuttons** for Start & Reset
- **Piezo buzzer**
- Optional: WiFi network and backend server

---

## 🛠 Wiring Summary

| Component   | Pins                                     |
| ----------- | ---------------------------------------- |
| TFT Display | CS=10, DC=9, RST=8, SPI pins             |
| Touchscreen | XP=9, XM=A3, YP=A2, YM=8                 |
| RGB LED     | R=5, G=6, B=7                            |
| Buttons     | Start=2 (interrupt), Reset=3 (interrupt) |
| Buzzer      | Pin 4                                    |

---

## 🌐 Network Features

If WiFi credentials (in `arduino_secrets.h`) are valid, the device will:

### POST Session Completion

```
POST /session
{
  "type": "focus" | "break",
  "duration_ms": <milliseconds>,
  "is_cycle_complete": 0 | 1
}
```

### GET Configuration

```
GET /config
→ returns JSON with:
  focus_ms
  short_break_ms
  long_break_ms
```

If WiFi disconnects, the device attempts a reconnect; otherwise it runs offline.

---

## ▶️ Running the Program

1. Install required Arduino libraries:

   - Adafruit_ILI9341
   - Adafruit_GFX
   - TouchScreen
   - WiFiS3

2. Add your WiFi credentials in `arduino_secrets.h`.

3. Compile and upload `Arduino-Pomodoro-Timer.ino`.

4. The home screen will appear on the TFT.

5. Press:

   - **START** (button or touchscreen) → begin cycle
   - **PAUSE/RESUME** → touchscreen
   - **RESET** → button or touchscreen

---

## 🧪 Fast Testing Mode

Set:

```cpp
bool fastTesting = true;
```

And the timer speeds become:

- Focus: 10 seconds
- Short Break: 5 seconds
- Long Break: 8 seconds

Useful for demos and debugging.

---

## 🐶 Watchdog Behavior

- Tracks expected end time of each phase
- If timer drifts more than tolerance → triggers:

```
NVIC_SystemReset();
```

Ensures real-time correctness.

---

## 🎵 RTTTL Music

Music strings such as:

```cpp
"FocusDone:d=4,o=6,b=180:8g,8b,8d7,4g7"
```

Are parsed into frequency/duration arrays and played on the buzzer.

---

## 📄 License

Created for educational use in **CSCI 1600 (Real-Time & Embedded Systems)** at Brown University.
