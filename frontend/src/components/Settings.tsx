import { useState } from "react";
import "../styles/settings.css";

export default function Settings() {
  // store minutes in state, not milliseconds
  const [focusMin, setFocusMin] = useState<string>("");
  const [shortBreakMin, setShortBreakMin] = useState<string>("");
  const [longBreakMin, setLongBreakMin] = useState<string>("");
  const [message, setMessage] = useState("");

  async function saveConfig() {
    const body: any = {};

    // convert minutes to ms
    if (focusMin !== "") body.focus_ms = Number(focusMin) * 60 * 1000;
    if (shortBreakMin !== "")
      body.short_break_ms = Number(shortBreakMin) * 60 * 1000;
    if (longBreakMin !== "")
      body.long_break_ms = Number(longBreakMin) * 60 * 1000;

    if (Object.keys(body).length === 0) {
      setMessage("Enter at least one field before saving.");
      return;
    }

    try {
      const res = await fetch("http://localhost:3000/update-config", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(body),
      });

      if (!res.ok) throw new Error("Request failed");

      setMessage("Settings updated!");
    } catch (err) {
      setMessage("Failed to update settings.");
    }
  }

  function resetFields() {
    setFocusMin("");
    setShortBreakMin("");
    setLongBreakMin("");
    setMessage("");
  }

  return (
    <main className="settings">
      <h1>Settings</h1>

      <div className="form">
        <div className="form-row">
          <label htmlFor="focus">Focus Duration (minutes)</label>
          <input
            id="focus"
            type="number"
            min="1"
            value={focusMin}
            placeholder="25"
            onChange={(e) => setFocusMin(e.target.value)}
          />
        </div>

        <div className="form-row">
          <label htmlFor="short-break">Short Break (minutes)</label>
          <input
            id="short-break"
            type="number"
            min="1"
            value={shortBreakMin}
            placeholder="5"
            onChange={(e) => setShortBreakMin(e.target.value)}
          />
        </div>

        <div className="form-row">
          <label htmlFor="long-break">Long Break (minutes)</label>
          <input
            id="long-break"
            type="number"
            min="1"
            value={longBreakMin}
            placeholder="15"
            onChange={(e) => setLongBreakMin(e.target.value)}
          />
        </div>
      </div>

      <div className="button-row">
        <button onClick={saveConfig}>Save</button>
        <button onClick={resetFields}>Reset</button>
      </div>

      {message && <p className="status">{message}</p>}
    </main>
  );
}
