# 🍅 LED Pomodoro Timer

### _An Embedded Real-Time Focus System with Arduino, Web Dashboard, and Data Analytics_

**CSCI 1600 – Real-Time & Embedded Systems | Final Project**

---

## 📌 Overview

The **LED Pomodoro Timer** is a complete real-time focus-tracking system built using:

- **Arduino Uno R4 WiFi** (hardware device)
- **Node.js + SQLite backend** (data storage + API)
- **React + TypeScript frontend** (web insights dashboard)

The system implements the Pomodoro technique:
**4 cycles of 25-minute focus sessions, each followed by a break**, culminating in a longer break.
The device logs data to a local backend, which the website uses to generate analytics and insights.

---

## 🧱 System Architecture

```
 ┌───────────────────────────────────────────────────────────┐
 │                         FRONTEND                           │
 │  React Dashboard (Insights, Settings, About)               │
 │  ├ Fetches /insights to display charts & stats             │
 │  ├ Posts /update-config to change durations                │
 │  └ Shows lifetime productivity metrics                     │
 └───────────────▲────────────────────────────────────────────┘
                 │  HTTP (fetch)
                 │
 ┌───────────────┴────────────────────────────────────────────┐
 │                         BACKEND                             │
 │      Node.js + Express + SQLite Database                    │
 │  ├ Stores session logs from Arduino via POST /session        │
 │  ├ Computes statistics via GET /insights                     │
 │  ├ Serves current durations via GET /config                  │
 │  └ Accepts updates via POST /update-config                   │
 └───────────────▲────────────────────────────────────────────┘
                 │  WiFi (client.connect)
                 │
 ┌───────────────┴────────────────────────────────────────────┐
 │                          ARDUINO                            │
 │   Physical Pomodoro device                                  │
 │  ├ TFT touchscreen UI                                       │
 │  ├ LEDs showing phase color                                 │
 │  ├ Buzzer w/ RTTTL melodies                                 │
 │  ├ Hardware interrupt buttons                               │
 │  ├ Watchdog timing + auto reset                             │
 │  └ Sends session results to backend over WiFi               │
 └─────────────────────────────────────────────────────────────┘
```

---

## 🔧 Components

### **1. Arduino Device**

Located in `/Arduino-Pomodoro-Timer/`

Features:

- TFT display with running timer
- Touchscreen start/pause/reset
- RGB LED showing phase state
- Buzzer with RTTTL melodies
- Hardware interrupts for start/reset buttons
- Watchdog timer for real-time correctness
- WiFi communication to backend
- Sends:

  - `"focus"` sessions
  - `"break"` sessions
  - Cycle completion flags

The Arduino fetches updated durations (`focus_ms`, etc.) from the backend.

---

### **2. Backend Server**

Located in `/backend/`

Tech stack:
**Node.js + Express + SQLite**

Handles:

#### Data logging

`POST /session`

#### Configuration

`GET /config`
`POST /update-config`

#### Insights Engine

`GET /insights` returns:

- Last 7 days of focus minutes
- Daily averages
- Today's focus/break time
- Lifetime focus/break minutes
- Lifetime session counts
- Completed Pomodoro cycles

Data is stored persistently in `pomodoro.db`.

---

### **3. Frontend Dashboard**

Located in `/frontend/`

Tech stack:
**React + TypeScript + Vite + React Router + Chart.js**

Pages:

#### 📊 **Insights**

- Focus/break totals
- Today's progress
- Completed cycles
- 7-day bar chart
- Daily average focus time

#### ⚙️ **Settings**

- Allows user to input durations **in minutes**
- Sends updates to `/update-config`
- Inputs are empty by default
- Backend stores updated durations
- Arduino picks them up on next WiFi sync / reboot

#### ℹ️ **About**

- Project purpose
- Pomodoro technique explanation
- Target audience

---

## ▶️ Running the Full System

### **Backend**

```bash
cd backend
npm install
node server.js
```

Starts API at:

```
http://localhost:3000
```

---

### **Frontend**

```bash
cd frontend
npm install
npm run dev
```

Opens:

```
http://localhost:5173
```

---

### **Arduino**

- Open project in Arduino IDE
- Add WiFi credentials to `arduino_secrets.h`
- Upload to Arduino Uno R4 WiFi
- Open Serial Monitor to confirm:

  - WiFi connection
  - Config fetch
  - Timer and watchdog behavior

---

## 🧪 Testing Workflow

1. Start backend
2. Visit frontend → Settings page
3. Enter minutes for Focus/Break/Long Break
4. Hit "Save"
5. Backend log should show updated config
6. Restart Arduino → Serial Monitor should show updated ms values
7. Run a few focus/break sessions
8. Visit Insights page → Chart and stats update

---

## 🧠 Key Design Principles

- **Real-time correctness**
  Using watchdog timing to detect drift.

- **Modular system**
  Hardware, backend, and frontend are decoupled but cooperate over HTTP.

- **User configurability**
  Durations dynamically adjustable through the dashboard.

- **Persistence**
  SQLite ensures all sessions survive restarts.

- **Human-centered design**
  Minimal distractions; physical-only focus device.

---

## 👩‍💻 Team

**Anna, Dani, Razan, Sydney**

---

## 📄 License

Created for educational purposes as part of **CSCI 1600 — Real-Time & Embedded Systems** at Brown University.
