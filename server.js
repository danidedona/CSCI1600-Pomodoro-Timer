const express = require("express");
const sqlite3 = require("sqlite3").verbose();
const cors = require("cors");

const app = express();
app.use(express.json());
app.use(cors()); // allow React frontend to access API

// initialize and open sqlite database
const db = new sqlite3.Database("pomodoro.db");

// create table first time around
db.run(`
  CREATE TABLE IF NOT EXISTS sessions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp INTEGER NOT NULL,
    type TEXT NOT NULL,
    duration_ms INTEGER NOT NULL,
    is_cycle_complete INTEGER NOT NULL
  )
`);

console.log("Database initialized.");

app.post("/session", (req, res) => {
  const { type, duration_ms, is_cycle_complete } = req.body;

  if (!type || !duration_ms) {
    return res.status(400).json({ error: "missing fields" });
  }

  const timestamp = Date.now();

  db.run(
    `INSERT INTO sessions (timestamp, type, duration_ms, is_cycle_complete)
     VALUES (?, ?, ?, ?)`,
    [timestamp, type, duration_ms, is_cycle_complete ? 1 : 0],
    function (err) {
      if (err) return res.status(500).json({ error: err.message });

      console.log("Inserted session:", {
        id: this.lastID,
        timestamp,
        type,
        duration_ms,
        is_cycle_complete,
      });

      res.json({ ok: true, id: this.lastID });
    }
  );
});

// get route to get all stored sessions for testing
app.get("/sessions", (req, res) => {
  db.all("SELECT * FROM sessions ORDER BY timestamp DESC", (err, rows) => {
    if (err) return res.status(500).json({ error: err.message });
    res.json(rows);
  });
});

// default durations (25m, 5m, 15m)
let pomodoroConfig = {
  focus_ms: 25 * 60 * 1000,
  short_break_ms: 5 * 60 * 1000,
  long_break_ms: 15 * 60 * 1000,
};

// post route to edit session durations
app.post("/update-config", (req, res) => {
  const { focus_ms, short_break_ms, long_break_ms } = req.body;

  // basic validation
  if (focus_ms !== undefined) pomodoroConfig.focus_ms = focus_ms;
  if (short_break_ms !== undefined)
    pomodoroConfig.short_break_ms = short_break_ms;
  if (long_break_ms !== undefined) pomodoroConfig.long_break_ms = long_break_ms;

  console.log("Updated pomodoro config:", pomodoroConfig);

  res.json({ ok: true, config: pomodoroConfig });
});

// get route to fetch session durations
app.get("/config", (req, res) => {
  res.json(pomodoroConfig);
});

app.listen(3000, "0.0.0.0", () => {
  console.log("Server running on port 3000 (LAN accessible)");
});
