import "./App.css";
import { useFirestoreDoc } from "./hooks/useFirestoreDoc";
import { useNavigate } from "react-router-dom";
import { StatisticsAccordion } from "./components/statisticsAccordion";

type ElevatorStatus = {
  currentFloor: number;
  maxFloor: number;
  minFloor: number;
};

export default function Dashboard() {
  const {
    data: statusData,
    loading: statusLoading,
    error: statusError,
  } = useFirestoreDoc<ElevatorStatus>("elevator", "1");

  const navigate = useNavigate();

  if (statusLoading) {
    return <div className="page">Loading…</div>;
  }

  if (statusError || !statusData) {
    return (
      <div className="page">
        <p>Error loading elevator status.</p>
      </div>
    );
  }

  let { currentFloor, maxFloor, minFloor } = statusData;
  currentFloor += minFloor;
  console.log("Elevator status:", statusData);

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
            <div className="cardBody status">
              <p>
                <span>
                  Current floor: <b>{currentFloor}</b>
                </span>
                <span>
                  Max floor: <b>{maxFloor}</b>
                </span>
                <span>
                  Min floor: <b>{minFloor}</b>
                </span>
              </p>
            </div>
            <div className="cardBody warning">
              <p className="green">No anomalies detected.</p>
              <p className="red">Temperature anomaly detected.</p>
              <p className="red">Acceleration anomaly detected.</p>
            </div>
          </section>
          <StatisticsAccordion />
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
        </div>

        <section className="elevator">
          {Array.from(
            { length: maxFloor - minFloor + 1 },
            (_, i) => maxFloor - i,
          ).map((floor) => (
            <div
              key={floor}
              className={`elevator-floor ${
                floor === currentFloor ? "active" : ""
              }`}
            >
              <p>Floor {floor}</p>
            </div>
          ))}
        </section>
      </main>
    </div>
  );
}
