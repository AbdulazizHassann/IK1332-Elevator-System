import "./App.css";
import { useRealtimeValue } from "./hooks/useRealtimeValue";
import type { ElevatorStatus } from "./types/elevator";

export default function App() {
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
      </main>
    </div>
  );
}
