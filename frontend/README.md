# 🌐 Pomodoro Timer Web Dashboard

A React + TypeScript frontend for visualizing Pomodoro productivity data, customizing timer settings, and providing an informational overview of the project.
This dashboard interfaces with the backend server at `http://localhost:3000` and displays insights collected from the Arduino Pomodoro Timer device.

---

## 🚀 Overview

The frontend provides a clean, responsive dashboard with:

### ✔️ **Productivity Insights**

Displays:

- Past 7 days of focus time (bar chart)
- Lifetime focus & break statistics
- Today's focus & break minutes
- Completed Pomodoro cycles
- Average daily focus time

### ✔️ **Settings Panel**

Allows users to:

- Update focus, short break, and long break durations
- Send configuration changes to the backend (`/update-config`)

### ✔️ **About Page**

Explains:

- Pomodoro technique
- Device purpose and audience
- Project background

### ✔️ **Navigation**

Uses React Router for:

- `/` → Insights
- `/settings`
- `/about`

---

## 📁 File Structure

```
frontend/
│
├── index.html
├── src/
│   ├── main.tsx
│   ├── App.tsx
│   ├── components/
│   │   ├── Insights.tsx
│   │   ├── Settings.tsx
│   │   ├── About.tsx
│   │   ├── BarChart.tsx
│   ├── styles/
│   │   ├── insights.css
│   │   ├── settings.css
│   │   ├── about.css
│   └── assets/
│       └── tomato.png
```

---

## 🧩 Key Files & Responsibilities

### **index.html**

- Bootstraps the React app
- Injects `<div id="root">`
- Loads frontend font + favicon

---

### **main.tsx**

- Mounts the React application
- Wraps the app in `BrowserRouter`
- Enables routing

---

### **App.tsx**

Defines the entire navigation structure:

- Global header + nav bar
- Routes:

  - `/` or `/insights` → `Insights`
  - `/settings` → `Settings`
  - `/about` → `About`

Includes the project logo (`tomato.png`).

---

### **Insights.tsx**

Fetches data from:

```
GET http://localhost:3000/insights
```

Displays:

- Total focus & break minutes
- Total lifetime sessions
- Today's stats
- Average minutes focused over the last 7 days
- Completed Pomodoro cycles
- **Bar chart visualization** of last 7 days of focus minutes

Uses the `BarChart` component.

---

### **BarChart.tsx**

A wrapper around `react-chartjs-2` + `Chart.js`.

Features:

- Custom font + styling
- No legend
- Clean gridlines
- Pink focus bars (`#ff4c51`)

Accepts:

```ts
labels: string[]
values: number[]
```

---

### **Settings.tsx**

Provides a UI for editing durations:

- Focus duration
- Short break duration
- Long break duration

UI elements are rendered, and can be wired to:

```
POST /update-config
```

---

### **About.tsx**

Static information page describing:

- What the Pomodoro device does
- Why the technique is useful
- Intended audience
- Team members

---

## ⚙️ Backend Communication

The frontend **expects** a backend running at:

```
http://localhost:3000
```

Endpoints used:

### ✔️ `GET /insights`

Displays aggregated productivity stats.

### ✔️ (Planned) `POST /update-config`

Will update session durations based on user input.

### ✔️ `GET /config`

(Optional) Can be used to pre-fill the settings form.

---

## ▶️ Running the Frontend

### 1. Install dependencies

```bash
npm install
```

### 2. Start the dev server

```bash
npm run dev
```

The app will be available at:

```
http://localhost:5173
```

### 3. Ensure backend is running

In a separate terminal:

```bash
npm start
```

This must be active for Insights and Settings to function.

---

## 🎨 Styling & Design Notes

- Pages use section-specific stylesheet files
- Custom typography loaded via Adobe Typekit (`CoFo Sans Pixel`)
- Layout uses a “bento grid” on the insights page
- Color palette matches the embedded device aesthetic (soft reds, pinks, and neutrals)

---

## 📄 License

Created for **CSCI 1600 – Real-Time & Embedded Systems**.
Designed to complement the physical Arduino Pomodoro Timer.
