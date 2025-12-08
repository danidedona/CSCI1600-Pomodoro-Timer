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

app.get("/insights", (req, res) => {
  const now = Date.now();
  const sevenDaysAgo = now - 7 * 24 * 60 * 60 * 1000;

  // get all  focus sessions from a certain timestamp cutoff, convert ms to a date, then group by day
  const qDays = `
    SELECT 
      DATE(timestamp/1000, 'unixepoch', 'localtime') AS day,
      SUM(duration_ms) AS total_ms
    FROM sessions
    WHERE type = 'focus'
      AND timestamp >= ?
    GROUP BY day
  `;

  // minutes focused today
  const qToday = `
    SELECT SUM(duration_ms) AS total_ms
    FROM sessions
    WHERE type='focus'
      AND DATE(timestamp/1000, 'unixepoch', 'localtime') = DATE('now', 'localtime')
  `;

  // minutes of break today
  const qTodayBreak = `
  SELECT SUM(duration_ms) AS total_ms
  FROM sessions
  WHERE type='break'
    AND DATE(timestamp/1000, 'unixepoch', 'localtime') = DATE('now', 'localtime')
`;

  // lifetime minutes focused
  const qLifetimeFocus = `
    SELECT SUM(duration_ms) AS total_ms FROM sessions WHERE type='focus'
  `;

  // lifetime minutes in break
  const qLifetimeBreak = `
    SELECT SUM(duration_ms) AS total_ms FROM sessions WHERE type='break'
  `;

  // lifetime num of focus sessions
  const qCountFocus = `
    SELECT COUNT(*) AS count FROM sessions WHERE type='focus'
  `;

  // lifetime num of break sessions
  const qCountBreak = `
    SELECT COUNT(*) AS count FROM sessions WHERE type='break'
  `;

  // lifetime number of completed pomodoro cycles
  const qCountCompletedCycles = `
  SELECT COUNT(*) AS count FROM sessions WHERE is_cycle_complete = 1
`;

  const insights = {
    last_7_days: [], // array of {day, minutes}
    last_7_days_total: 0,
    last_7_days_avg: 0,
    today_focus_min: 0,
    today_break_min: 0,
    lifetime_focus_min: 0,
    lifetime_break_min: 0,
    lifetime_focus_count: 0,
    lifetime_break_count: 0,
    lifetime_completed_cycles: 0,
  };

  // get focus data for past 7 days
  // pass in sevenDaysAgo as timestamp
  db.all(qDays, [sevenDaysAgo], (err, rowsDays) => {
    if (err) return res.status(500).json({ error: err.message });

    // loop backwards from 6 days ago until today
    for (let i = 6; i >= 0; i--) {
      // get date for i days ago
      const d = new Date(now - i * 24 * 60 * 60 * 1000);
      const dayStr = d.toISOString().slice(0, 10); // YYYY-MM-DD

      const match = rowsDays.find((r) => r.day === dayStr);

      const minutes = match ? match.total_ms / 60000 : 0;

      insights.last_7_days.push({ day: dayStr, minutes });
    }

    // compute total focus time for past 7 days
    insights.last_7_days_total = insights.last_7_days.reduce(
      (sum, item) => sum + item.minutes,
      0
    );
    // compute avg focus time per day for last 7 days using total
    insights.last_7_days_avg =
      insights.last_7_days_total / insights.last_7_days.length;

    // get time focused for today
    db.get(qToday, [], (err, todayRow) => {
      if (err) return res.status(500).json({ error: err.message });

      insights.today_focus_min = todayRow.total_ms
        ? todayRow.total_ms / 60000
        : 0;

      db.get(qTodayBreak, [], (err, todayBreakRow) => {
        if (err) return res.status(500).json({ error: err.message });

        insights.today_break_min = todayBreakRow.total_ms
          ? todayBreakRow.total_ms / 60000
          : 0;

        // get lifetime time spent focused
        db.get(qLifetimeFocus, [], (err, lfRow) => {
          if (err) return res.status(500).json({ error: err.message });

          insights.lifetime_focus_min = lfRow.total_ms
            ? lfRow.total_ms / 60000
            : 0;

          // get lifetime time spent in break
          db.get(qLifetimeBreak, [], (err, lbRow) => {
            if (err) return res.status(500).json({ error: err.message });

            insights.lifetime_break_min = lbRow.total_ms
              ? lbRow.total_ms / 60000
              : 0;

            // get lifetime # of focus sessions
            db.get(qCountFocus, [], (err, c1) => {
              if (err) return res.status(500).json({ error: err.message });

              insights.lifetime_focus_count = c1.count;

              // get lifetime # of break sessions
              db.get(qCountBreak, [], (err, c2) => {
                if (err) return res.status(500).json({ error: err.message });

                insights.lifetime_break_count = c2.count;

                db.get(qCountCompletedCycles, [], (err, c3) => {
                  if (err) return res.status(500).json({ error: err.message });

                  insights.lifetime_completed_cycles = c3.count;

                  // finally return insights
                  res.json(insights);
                });
              });
            });
          });
        });
      });
    });
  });
});

app.listen(3000, "0.0.0.0", () => {
  console.log("Server running on port 3000 (LAN accessible)");
});
