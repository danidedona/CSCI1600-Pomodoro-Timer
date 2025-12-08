import "../styles/settings.css";

export default function Settings() {
  return (
    <main className="settings">
      <h1>Settings</h1>
      <div className="form">
        <div className="form-row">
          <label htmlFor="focus">Focus Duration (ms)</label>
          <input id="focus" type="number" min="1" />
        </div>
        <div className="form-row">
          <label htmlFor="short-break">Short Break Duration (ms)</label>
          <input id="short-break" type="number" min="1" />
        </div>
        <div className="form-row">
          <label htmlFor="long-break">Long Break Duration (ms)</label>
          <input id="long-break" type="number" min="1" />
        </div>
      </div>
      <div className="button-row">
        <button id="save">Save</button>
        <button id="reset">Reset</button>
      </div>
    </main>
  );
}
