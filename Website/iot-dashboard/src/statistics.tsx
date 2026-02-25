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
  BarChart,
  Bar,
  Legend,
} from "recharts";
import { useReadingsHistory } from "./hooks/useReadingsHistory";

type Sensor =
  | "temperature"
  | "pressure"
  | "acceleration"
  | "gyroscope"
  | "magnetometer"
  | "travel";

type Reading = { timestamp: Timestamp } & Record<string, number>;
type TravelReading = { timestamp: Timestamp; floor: number };

type ChartPoint =
  | { time: string; value: number }
  | { time: string; x: number; y: number; z: number };

type TravelPoint = { time: string; floor: number };

const SENSOR: Record<Sensor, { label: string; unit: string }> = {
  temperature: { label: "Temperature", unit: "°C" },
  pressure: { label: "Pressure", unit: "Pa" },
  acceleration: { label: "Acceleration", unit: "m/s²" },
  gyroscope: { label: "Gyroscope", unit: "°/s" },
  magnetometer: { label: "Magnetometer", unit: "µT" },
  travel: { label: "Travel", unit: "floor" },
};

const fmtTime = (t?: Timestamp) =>
  t?.toDate?.().toLocaleTimeString([], {
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit",
  }) ?? "";

export default function Statistics() {
  const { sensor: sensorParam } = useParams();
  const navigate = useNavigate();

  const sensor = (sensorParam ?? "") as Sensor;
  const meta = SENSOR[sensor];

  const isTravel = sensor === "travel";

  const subcollection = meta ? (isTravel ? "travelHistory" : sensor) : "";

  const {
    data: readingsRaw = [],
    loading,
    error,
  } = useReadingsHistory<Reading | TravelReading>("1", subcollection, {
    limit: 100,
    orderField: "timestamp",
  });

  const sensorData: ChartPoint[] =
    !isTravel && meta
      ? (readingsRaw as Reading[])
          .slice()
          .reverse() // query is desc -> show chronological
          .map((r) => {
            const time = fmtTime(r.timestamp);
            return sensor === "gyroscope"
              ? { time, x: r.x, y: r.y, z: r.z }
              : { time, value: r[sensor] };
          })
      : [];

  const travelData: TravelPoint[] = isTravel
    ? (readingsRaw as TravelReading[])
        .slice()
        .reverse()
        .map((r) => ({
          time: fmtTime(r.timestamp),
          floor: r.floor,
        }))
    : [];

  return (
    <div className="page">
      <header className="topbar">
        <div>
          <div className="title">
            {(sensorParam ?? "SENSOR").toUpperCase()} Statistics
          </div>
          <div className="subtitle">Historical Data</div>
        </div>
        <div className="accordion-header">
          <button onClick={() => navigate("/")}>Back</button>
        </div>
      </header>

      <main style={{ padding: 40 }}>
        <div className="card">
          <div className="cardTitle">Graph Preview</div>
          <div className="cardBody">
            {loading && <div>Loading…</div>}
            {error && <div>Error: {error.message}</div>}
            {!meta && !loading && (
              <div>Unknown sensor: {String(sensorParam)}</div>
            )}

            {!loading && !error && meta && (
              <ResponsiveContainer width="100%" height={350}>
                {isTravel ? (
                  <BarChart data={travelData} barCategoryGap="0%" barGap={0}>
                    <text
                      x="50%"
                      y="20"
                      textAnchor="middle"
                      style={{
                        fontSize: 16,
                        fontWeight: 600,
                      }}
                    >
                      Travel (Floor Over Time)
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
                      domain={[1, 7]}
                      ticks={[2, 3, 4, 5, 6, 7]}
                      allowDecimals={false}
                      label={{
                        value: "Floor",
                        angle: -90,
                        position: "insideLeft",
                      }}
                    />
                    <Tooltip cursor={{ fill: "rgba(76, 175, 80, 0.15)" }} />
                    <Bar dataKey="floor" fill="#4caf50" />
                  </BarChart>
                ) : (
                  <LineChart data={sensorData}>
                    <text
                      x="50%"
                      y="20"
                      textAnchor="middle"
                      style={{ fontSize: 16, fontWeight: 600 }}
                    >
                      {meta.label} Over Time
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
                        value: `${meta.label} (${meta.unit})`,
                        angle: -90,
                        position: "insideLeft",
                      }}
                    />

                    <Tooltip
                      formatter={(value: any, name: any) =>
                        sensor === "gyroscope"
                          ? [
                              `${value} ${meta.unit}`,
                              String(name).toUpperCase(),
                            ]
                          : `${value} ${meta.unit}`
                      }
                    />

                    {sensor === "gyroscope" && <Legend />}

                    {sensor === "gyroscope" ? (
                      <>
                        <Line
                          type="monotone"
                          dataKey="x"
                          strokeWidth={2}
                          dot={false}
                          stroke="#f44336"
                        />
                        <Line
                          type="monotone"
                          dataKey="y"
                          strokeWidth={2}
                          dot={false}
                          stroke="#2196f3"
                        />
                        <Line
                          type="monotone"
                          dataKey="z"
                          strokeWidth={2}
                          dot={false}
                          stroke="#4caf50"
                        />
                      </>
                    ) : (
                      <Line
                        type="monotone"
                        dataKey="value"
                        strokeWidth={3}
                        dot={{ r: 4 }}
                      />
                    )}
                  </LineChart>
                )}
              </ResponsiveContainer>
            )}
          </div>
        </div>
      </main>
    </div>
  );
}
