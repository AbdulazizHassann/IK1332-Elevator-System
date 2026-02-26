import "./App.css";
import { useFirestoreDoc } from "./hooks/useFirestoreDoc";
import { StatisticsAccordion } from "./components/statisticsAccordion";

// Type definition for elevator data from Firestore
type ElevatorStatus = {
  currentFloor: number;
  maxFloor: number;
  minFloor: number;
  anomalyAcceleration: boolean;
  anomalyTemperature: boolean;
  isMoving: boolean;
};

export default function Dashboard() {
  // Fetch elevator document ("elevator", id "1") from Firestore
  const {
    data: statusData,
    loading: statusLoading,
    error: statusError,
  } = useFirestoreDoc<ElevatorStatus>("elevator", "1");

  // Show loading state while waiting for data
  if (statusLoading) {
    return <div className="page">Loading…</div>;
  }
  // Show error message if something goes wrong
  if (statusError || !statusData) {
    return (
      <div className="page">
        <p>Error loading elevator status.</p>
      </div>
    );
  }
   // Destructure elevator values from Firestore data
  let {
    currentFloor,
    maxFloor,
    minFloor,
    anomalyAcceleration,
    anomalyTemperature,
    isMoving,
  } = statusData;
  // For debugg purposes.
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
                  Moving: <b>{isMoving ? "Yes" : "No"}</b>
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
              {!anomalyTemperature && !anomalyAcceleration && (
                <p className="green">No anomalies detected.</p>
              )}
              {anomalyTemperature && (
                <p className="red">Temperature anomaly detected.</p>
              )}
              {anomalyAcceleration && (
                <p className="red">Acceleration anomaly detected.</p>
              )}
            </div>
          </section>
          <StatisticsAccordion />
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
