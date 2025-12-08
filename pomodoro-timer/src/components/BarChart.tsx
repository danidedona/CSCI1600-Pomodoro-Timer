import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  BarElement,
  Tooltip,
  Legend,
} from "chart.js";
import { Bar } from "react-chartjs-2";

ChartJS.register(CategoryScale, LinearScale, BarElement, Tooltip, Legend);

ChartJS.defaults.font.family = "'CoFo Sans Pixel', sans-serif"; // or any font
ChartJS.defaults.font.size = 24;
ChartJS.defaults.color = "black";

type chartProps = {
  labels: string[];
  values: number[];
};

export default function BarChart({ labels, values }: chartProps) {
  const data = {
    labels,
    datasets: [
      {
        label: "Focus minutes",
        data: values,
        backgroundColor: "#ff4c51",
        borderRadius: 0,
        borderWidth: 1,
      },
    ],
  };

  const options = {
    responsive: true,
    plugins: {
      legend: {
        display: false,
      },
    },
    scales: {
      x: {
        grid: { display: false },
      },
      y: {
        grid: { color: "rgba(0,0,0,0.1)" },
        ticks: { stepSize: 5 },
      },
    },
  };

  return <Bar data={data} options={options} />;
}
