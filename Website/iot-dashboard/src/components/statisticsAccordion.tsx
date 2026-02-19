import { useNavigate } from "react-router-dom";
import { useState } from "react";
import { useLatestReading } from "../hooks/useLatestReading";
import { Timestamp } from "firebase/firestore";

type Statistic = {
  key: string;
  unit: string;
  data: number | string | null | undefined;
};

export function StatisticsAccordion() {
  const temperatures = useLatestReading<{
    temperature: number;
    timestamp: Timestamp;
  }>("1", "temperatures");
  const pressure = useLatestReading<{
    pressure: number;
    timestamp: Timestamp;
  }>("1", "pressure");
  const acceleration = useLatestReading<{
    acceleration: number;
    timestamp: Timestamp;
  }>("1", "acceleration");
  const gyroscope = useLatestReading<{
    x: number;
    y: number;
    z: number;
    timestamp: Timestamp;
  }>("1", "gyroscope");
  const magnetometer = useLatestReading<{
    magnetometer: number;
    timestamp: Timestamp;
  }>("1", "magnetometer");

  const navigate = useNavigate();
  const [openSensor, setOpenSensor] = useState<string | null>(null);
  console.log("updating");

  const statistics: Statistic[] = [
    {
      key: "Temperature",
      unit: "°C",
      data: temperatures.data?.temperature,
    },
    {
      key: "Pressure",
      unit: "Pa",
      data: pressure.data?.pressure,
    },
    {
      key: "Acceleration",
      unit: "m/s²",
      data: acceleration.data?.acceleration,
    },
    {
      key: "Gyroscope",
      unit: "°/s",
      data: gyroscope.data
        ? `(x: ${gyroscope.data.x}, y: ${gyroscope.data.y}, z: ${gyroscope.data.z})`
        : null,
    },
    {
      key: "Magnetometer",
      unit: "µT",
      data: magnetometer.data?.magnetometer,
    },
    { key: "Traffic", unit: "", data: null },
  ];

  return (
    <section className="card">
      <div className="cardTitle">Live Sensor Data</div>

      {statistics.map((stat) => {
        const isOpen = openSensor === stat.key;
        return (
          <div key={stat.key} className="accordion-item">
            <button
              type="button"
              className="accordion-header"
              onClick={() => setOpenSensor(isOpen ? null : stat.key)}
            >
              {isOpen ? "▼" : "▶"} {stat.key}
            </button>

            {isOpen && (
              <div className="accordion-content">
                <div className="accordion-header">
                  <p>
                    Current value: {stat.data ?? "—"} {stat.unit}
                  </p>
                  <button
                    type="button"
                    className="history-button"
                    onClick={() =>
                      navigate(`/statistics/${stat.key.toLowerCase()}`)
                    }
                  >
                    View Full History
                  </button>
                </div>
              </div>
            )}
          </div>
        );
      })}
    </section>
  );
}
