import "../styles/insights.css";
import { useEffect, useState } from "react";
import BarChart from "./BarChart";

type insightsResponse = {
  last_7_days: { day: string; minutes: number }[];
  last_7_days_total: number;
  last_7_days_avg: number;
  today_focus_min: number;
  today_break_min: number;
  lifetime_focus_min: number;
  lifetime_break_min: number;
  lifetime_focus_count: number;
  lifetime_break_count: number;
  lifetime_completed_cycles: number;
};

export default function Insights() {
  const [insights, setInsights] = useState<insightsResponse | null>(null);

  useEffect(() => {
    async function loadInsights() {
      try {
        const res = await fetch("http://localhost:3000/insights");
        if (!res.ok) {
          throw new Error("Failed to fetch /insights");
        }
        const json: insightsResponse = await res.json();

        console.log(json);

        setInsights(json);
      } catch (err: any) {
        throw new Error("Failed to fetch /insights");
      } finally {
      }
    }

    loadInsights();
  }, []);

  return (
    <main className="insights">
      <h1>Insights</h1>
      {insights && (
        <div className="bento-grid">
          <div className="box focus">
            <p className="focus-break-left">
              Total
              <br />
              Focus
            </p>
            <div className="focus-break-right">
              <div className="right-stat">
                <p className="right-stat-num">
                  {Math.round(insights.lifetime_focus_min)}
                </p>
                <p>minutes</p>
              </div>
              <div className="right-stat">
                <p className="right-stat-num">
                  {insights.lifetime_focus_count}
                </p>
                <p>sessions</p>
              </div>
            </div>
          </div>
          <div className="box break">
            <p className="focus-break-left">
              Total
              <br />
              Break
            </p>
            <div className="focus-break-right">
              <div className="right-stat">
                <p className="right-stat-num">
                  {Math.round(insights.lifetime_break_min)}
                </p>
                <p>minutes</p>
              </div>
              <div className="right-stat">
                <p className="right-stat-num">
                  {insights.lifetime_break_count}
                </p>
                <p>sessions</p>
              </div>
            </div>{" "}
          </div>
          <div className="box today">
            <p className="today-title">Today's Progress</p>
            <div className="today-bottom">
              <div className="today-stat">
                <p className="today-stat-num">
                  {Math.round(insights.today_focus_min)}
                </p>
                <p>focus minutes</p>
              </div>
              <div className="today-stat">
                <p className="today-stat-num">
                  {Math.round(insights.today_break_min)}
                </p>
                <p>break minutes</p>
              </div>
            </div>
          </div>
          <div className="box avg">
            <p className="avg-cycles-num">
              {Math.round(insights.last_7_days_avg * 10) / 10}
            </p>
            <p className="avg-cycles-label">
              minutes <br />
              focused daily <br /> on average
            </p>
          </div>
          <div className="box cycles">
            <p className="avg-cycles-num">
              {insights.lifetime_completed_cycles}
            </p>
            <p className="avg-cycles-label">
              completed
              <br /> pomodoro
              <br /> cycles
            </p>
          </div>
          <div className="box chart">
            <p className="chart-title">Focus Minutes Over the Last 7 Days</p>
            <BarChart
              labels={insights.last_7_days.map((d) => {
                const [_, month, day] = d.day.split("-");
                return `${month}/${day}`;
              })}
              values={insights.last_7_days.map((d) => Math.round(d.minutes))}
            />
          </div>
        </div>
      )}
    </main>
  );
}
