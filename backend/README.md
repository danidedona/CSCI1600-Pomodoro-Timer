# 🖥️ Pomodoro Backend Server

Backend API for storing Pomodoro session data, calculating productivity insights, and serving configurable timer durations.
Designed as the companion server for the **Arduino Pomodoro Timer** in CSCI 1600.

This service exposes REST endpoints used by both the Arduino device and a web dashboard.

---

## 🚀 Overview

This backend provides:

- **Data storage** using SQLite (`pomodoro.db`)
- **POST endpoints** for logging completed focus/break sessions
- **GET endpoints** for retrieving all sessions, insights, and configuration
- **Runtime-editable Pomodoro durations**
- **Cross-origin access** for React or Arduino clients
- **LAN-accessible API** (listens on `0.0.0.0:3000`)

The server runs locally and persists all session data in a single lightweight database.

---

## 📁 File Contents

### **server.js**

Main Node.js application responsible for:

- Initializing SQLite and creating tables
- Handling all REST API routes
- Computing analytics (daily totals, averages, lifetime stats)
- Serving configurable timer durations to the Arduino
- Accepting focus/break session logs from the microcontroller
- Responding to frontend dashboard requests
- Allowing CORS access from web or local Arduino WiFi

No other files are required—`pomodoro.db` is automatically created if missing.

---

## 🗄️ Database Schema

SQLite file: **pomodoro.db**

Table: **sessions**

| Column            | Type    | Description                                       |
| ----------------- | ------- | ------------------------------------------------- |
| id                | INTEGER | Auto-increment primary key                        |
| timestamp         | INTEGER | Unix ms when the session completed                |
| type              | TEXT    | `"focus"` or `"break"`                            |
| duration_ms       | INTEGER | How long the session lasted                       |
| is_cycle_complete | INTEGER | `1` if completing the 4th focus session, else `0` |

The DB is appended to—records persist across restarts automatically.

---

## 🌐 API Endpoints

### **POST `/session`**

Logs a completed Pomodoro session.

#### Example Body

```json
{
  "type": "focus",
  "duration_ms": 1500000,
  "is_cycle_complete": 0
}
```

#### Response

```json
{ "ok": true, "id": 17 }
```

---

### **GET `/sessions`**

Returns all stored sessions, newest first.

Used for debugging or history visualizations.

---

### **GET `/config`**

Returns current Pomodoro durations:

```json
{
  "focus_ms": 1500000,
  "short_break_ms": 300000,
  "long_break_ms": 900000
}
```

These values are fetched by the Arduino on boot.

---

### **POST `/update-config`**

Updates Pomodoro durations at runtime.

#### Example

```json
{
  "focus_ms": 2000000,
  "short_break_ms": 300000
}
```

Only provided keys are updated.

---

### **GET `/insights`**

Returns an aggregated stats structure with:

- Last 7 days of focus time (per day)
- Total focus minutes last 7 days
- Average daily focus
- Focus + break time today
- Lifetime total focus + break minutes
- Lifetime counts of each session type
- Lifetime completed Pomodoro cycles

#### Response Structure

```json
{
  "last_7_days": [ { "day": "YYYY-MM-DD", "minutes": 42 }, ... ],
  "last_7_days_total": 240,
  "last_7_days_avg": 34.2,
  "today_focus_min": 25,
  "today_break_min": 5,
  "lifetime_focus_min": 600,
  "lifetime_break_min": 200,
  "lifetime_focus_count": 40,
  "lifetime_break_count": 30,
  "lifetime_completed_cycles": 10
}
```

This powers the dashboard analytics.

---

## ▶️ Running the Server

### 1. Install dependencies

```bash
npm install
```

### 2. Start the server

```bash
npm start
```

The server listens on:

```
http://localhost:3000
```

Accessible from your LAN due to:

```js
app.listen(3000, "0.0.0.0");
```

---

## 🔐 Notes on Persistence

- `pomodoro.db` is automatically created if missing.
- Data persists between runs without any manual saving required.
- Add the database file to **.gitignore** so it is never overwritten:

```
pomodoro.db
```

---

## 🧠 Architectural Summary

| Responsibility    | Implemented In                                     |
| ----------------- | -------------------------------------------------- |
| Database setup    | SQLite (`db.run(...)`)                             |
| POST logging      | `/session`                                         |
| Session retrieval | `/sessions`                                        |
| Config handling   | `/config`, `/update-config`                        |
| Insights engine   | `/insights` (SQL aggregation + JS post-processing) |
| CORS support      | `app.use(cors())`                                  |
| JSON body parsing | `app.use(express.json())`                          |

---

## 📄 License

Created as part of **CSCI 1600: Real-Time & Embedded Software**.
Intended for educational use with the accompanying Arduino hardware timer.
