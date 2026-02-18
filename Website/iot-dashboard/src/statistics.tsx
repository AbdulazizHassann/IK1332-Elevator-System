import { useParams, useNavigate } from "react-router-dom";
import { useEffect, useState } from "react";

import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  Tooltip,
  CartesianGrid,
  ResponsiveContainer
} from "recharts";

type DataPoint = {
  time: string;
  value: number;
};
const unitMap: Record<string, string> = {
  temperature: "°C",
  pressure: "Pa",
  acceleration: "m/s²",
  gyro: "°/s",
  magnetometer: "µT",
  traffic: "count",
};

const labelMap: Record<string, string> = {
  temperature: "Temperature",
  pressure: "Pressure",
  acceleration: "Acceleration",
  gyro: "Gyroscope",
  magnetometer: "Magnetometer",
  traffic: "Traffic",
};



export default function Statistics() {
  const { sensor } = useParams();
  const navigate = useNavigate();

  const [data, setData] = useState<DataPoint[]>([]);

  const unit = unitMap[sensor ?? ""] || "";
  const label = labelMap[sensor ?? ""] || "Sensor";

  // Fake test data  , replace with real historical data from the backend when available
  useEffect(() => {
    const fakeData: DataPoint[] = [
      { time: "10:00", value: 20 },
      { time: "10:01", value: 22 },
      { time: "10:02", value: 21 },
      { time: "10:03", value: 23 },
      { time: "10:04", value: 24 },
    ];

    setData(fakeData);
  }, [sensor]);

  return (
    <div className="page">
      <header className="topbar">
        <div>
          <div className="title">
            {sensor?.toUpperCase()} Statistics
          </div>
          <div className="subtitle">Historical Data</div>
        </div>
        <div className = "accordion-header">
        <button onClick={() => navigate("/")}>
          Back
        </button>
        </div>
      </header>

      <main style={{ padding: "40px" }}>
        <div className="card">
          <div className="cardTitle">Graph Preview</div>
          <div className="cardBody">
            
            {/* Fake graph visualization, but replace to the real data from sensors */}
            <ResponsiveContainer width="100%" height={350}>
              <LineChart data={data}>

                {/* TITLE IN GRAPH */}
                <text
                  x="50%"
                  y="20"
                  textAnchor="middle"
                  style={{ fontSize: "16px", fontWeight: 600 }}
                >
                  {label} Over Time
                </text>

                <CartesianGrid strokeDasharray="3 3" />

                <XAxis
                  dataKey="time"
                  label={{
                    value: "Time",
                    position: "insideBottom",
                    offset: -5,
                  }}
                />

                <YAxis
                  label={{
                    value: `${label} (${unit})`,
                    angle: -90,
                    position: "insideLeft",
                  }}
                />


                <Tooltip
                formatter={(value) => `${value} ${unit}`}

                />

                <Line
                  type="monotone"
                  dataKey="value"
                  stroke="#4caf50"
                  strokeWidth={3}
                  dot={{ r: 4 }}
                />

              </LineChart>
            </ResponsiveContainer>



          </div>
        </div>
      </main>
    </div>
  );
}
