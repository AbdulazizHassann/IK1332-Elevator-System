import "./App.css";
import { useRealtimeValue } from "./hooks/useRealtimeValue";
import type { ElevatorStatus } from "./types/elevator";
import { useNavigate } from "react-router-dom";
import { useState } from "react";


export default function Dashboard() {

  const navigate = useNavigate();

  const [openSensor, setOpenSensor] = useState<string | null>(null);

  const sensors = [
  { key: "temperature", label: "Temperature" },
  { key: "pressure", label: "Pressure" },
  { key: "acceleration", label: "Acceleration" },
  { key: "gyro", label: "Gyro" },
  { key: "magnetometer", label: "Magnetometer" },
  { key: "traffic", label: "Traffic" },
];

const unitMap: Record<string, string> = {
  temperature: "°C",
  pressure: "Pa",
  acceleration: "m/s²",
  gyro: "°/s",
  magnetometer: "µT",
  traffic: "",
};



  const {
    data: status,
    loading,
    error,
  } = useRealtimeValue<ElevatorStatus>("elevator/status");

  if (loading) {
    return <div className="page">Loading…</div>;
  }

  if (error || !status) {
    return (
      <div className="page">
        <p>Error loading elevator status.</p>
      </div>
    );
  }

  const { currentFloor, totalFloors } = status;



  return (
    <div className="page">
      <header className="topbar">
        <div>
          <div className="title">Dashboard</div>
          <div className="subtitle">IK1332 Elevator Project</div>
        </div>
      </header>

      <main className="grid">
        <div className="stats">
          <section className="about">
            <div className="cardTitle">About</div>
            <div className="cardBody">
              <p>
                This dashboard is a part of the IK1332 Elevator project. Created
                by group 2 which includes Abdulaziz Farah, Mickhel Sumarlie and
                Dante Nilsson.
              </p>
            </div>
          </section>

          <section className="card">
            <div className="cardTitle">Status</div>
            <div className="cardBody">
              <p>
                <b>Current floor:</b> {currentFloor}
              </p>
              <p>
                <b>Number of floors:</b> {totalFloors}
              </p>
            </div>
          </section>
        

        
          <section className="card">
            
          <div className="cardTitle">Live Sensor Data</div>

          {sensors.map((sensor) => (
            <div key={sensor.key} className="accordion-item">

              <div
                className="accordion-header"
                onClick={() =>
                  setOpenSensor(openSensor === sensor.key ? null : sensor.key)
                }
              >
                {openSensor === sensor.key ? "▼" : "▶"} {sensor.label}
              </div>

              {openSensor === sensor.key && (
                <div className="accordion-content">
                  <p>
                    <b>Current value:</b>{" "}
                    {status[sensor.key as keyof ElevatorStatus]}{" "}
                    {unitMap[sensor.key]}
                  </p>

                  <div className = "accordion-header">
                  <button
                    className="history-button"
                    onClick={() => navigate(`/statistics/${sensor.key}`)}
                  >
                    View Full History
                  </button>
                  </div>
                </div>
            )}

          </div>
        ))}
      </section>
      </div>

     <section className="elevator">
          {Array.from({ length: totalFloors }, (_, i) => totalFloors - i).map(
            (floor) => (
              <div
                key={floor}
                className={`elevator-floor ${
                  floor === currentFloor ? "active" : ""
                }`}
              >
                <p>Floor {floor}</p>
              </div>
            ),
          )}
        </section>


         
         

          <section className="warning-section">
                <div className="warning-header">Warning History</div>

                <select
                        onChange={(e) => {
                        if (e.target.value) {
                            navigate(`/statistics/warning/${e.target.value}`);
                        }
                        }}
                    >
                        <option value="">Select Warning Block</option>
                        <option value="block1">Block 1 - 12:30</option>
                        <option value="block2">Block 2 - 14:15</option>
                        <option value="block3">Block 3 - 18:40</option>
                </select>
         </section>



      </main>
    </div>
  );
}
