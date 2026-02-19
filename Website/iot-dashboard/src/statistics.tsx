import { useMemo } from "react";
import { useParams, useNavigate } from "react-router-dom";
import { Timestamp } from "firebase/firestore";

import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  Tooltip,
  CartesianGrid,
  ResponsiveContainer,
} from "recharts";

import { useReadingsHistory } from "./hooks/useReadingsHistory";

type DataPoint = {
  time: string;
  value: number;
};

const unitMap: Record<string, string> = {
  temperature: "°C",
  pressure: "Pa",
  acceleration: "m/s²",
  gyroscope: "°/s",
  magnetometer: "µT",
  traffic: "count",
};

const labelMap: Record<string, string> = {
  temperature: "Temperature",
  pressure: "Pressure",
  acceleration: "Acceleration",
  gyroscope: "Gyroscope",
  magnetometer: "Magnetometer",
  traffic: "Traffic",
};

const collectionMap: Record<string, string> = {
  temperature: "temperatures",
  pressure: "pressure",
  acceleration: "acceleration",
  gyroscope: "gyroscope",
  magnetometer: "magnetometer",
  traffic: "traffic",
};

type ReadingBase = { timestamp: Timestamp };

type TemperatureReading = ReadingBase & { temperature: number };
type PressureReading = ReadingBase & { pressure: number };
type AccelerationReading = ReadingBase & { acceleration: number };
type MagnetometerReading = ReadingBase & { magnetometer: number };
type TrafficReading = ReadingBase & { traffic: number };
type GyroReading = ReadingBase & { x: number; y: number; z: number };

export default function Statistics() {
  const { sensor } = useParams();
  const navigate = useNavigate();

  const unit = unitMap[sensor ?? ""] || "";
  const label = labelMap[sensor ?? ""] || "Sensor";
  const subcollection = collectionMap[sensor ?? ""] || "";

  const result = (() => {
    switch (sensor) {
      case "temperature":
        return useReadingsHistory<TemperatureReading>("1", subcollection, {
          limit: 100,
          orderField: "timestamp",
        });
      case "pressure":
        return useReadingsHistory<PressureReading>("1", subcollection, {
          limit: 100,
          orderField: "timestamp",
        });
      case "acceleration":
        return useReadingsHistory<AccelerationReading>("1", subcollection, {
          limit: 100,
          orderField: "timestamp",
        });
      case "magnetometer":
        return useReadingsHistory<MagnetometerReading>("1", subcollection, {
          limit: 100,
          orderField: "timestamp",
        });
      case "traffic":
        return useReadingsHistory<TrafficReading>("1", subcollection, {
          limit: 100,
          orderField: "timestamp",
        });
      case "gyroscope":
        return useReadingsHistory<GyroReading>("1", subcollection, {
          limit: 100,
          orderField: "timestamp",
        });
      default:
        console.error("Unknown sensor:", sensor);
        return {
          data: [],
          loading: false,
          error: new Error("Unknown sensor"),
        } as const;
    }
  })();

  const { data: readings, loading, error } = result;

  const data: DataPoint[] = useMemo(() => {
    if (!sensor) return [];

    // our query is "desc", so reverse to chronological order.
    const ordered = [...readings].reverse();

    return ordered.map((r: any) => {
      const time = r.timestamp?.toDate
        ? r.timestamp.toDate().toLocaleTimeString([], {
            hour: "2-digit",
            minute: "2-digit",
            second: "2-digit",
          })
        : "";

      let value = 0;

      switch (sensor) {
        case "temperature":
          value = r.temperature;
          break;
        case "pressure":
          value = r.pressure;
          break;
        case "acceleration":
          value = r.acceleration;
          break;
        case "magnetometer":
          value = r.magnetometer;
          break;
        case "traffic":
          value = r.traffic;
          break;
        case "gyroscope":
          // Single line needs one value, so we plot magnitude
          // Change it to three graphs?
          value = Math.sqrt(r.x * r.x + r.y * r.y + r.z * r.z);
          break;
      }

      return { time, value };
    });
  }, [readings, sensor]);

  return (
    <div className="page">
      <header className="topbar">
        <div>
          <div className="title">{sensor?.toUpperCase()} Statistics</div>
          <div className="subtitle">Historical Data</div>
        </div>
        <div className="accordion-header">
          <button onClick={() => navigate("/")}>Back</button>
        </div>
      </header>

      <main style={{ padding: "40px" }}>
        <div className="card">
          <div className="cardTitle">Graph Preview</div>
          <div className="cardBody">
            {loading && <div>Loading…</div>}
            {error && <div>Error: {error.message}</div>}

            {!loading && !error && (
              <ResponsiveContainer width="100%" height={350}>
                <LineChart data={data}>
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

                  <Tooltip formatter={(value) => `${value} ${unit}`} />

                  <Line
                    type="monotone"
                    dataKey="value"
                    stroke="#4caf50"
                    strokeWidth={3}
                    dot={{ r: 4 }}
                  />
                </LineChart>
              </ResponsiveContainer>
            )}
          </div>
        </div>
      </main>
    </div>
  );
}
