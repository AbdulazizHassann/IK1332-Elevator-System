import "./App.css";

export default function App() {
  return (
    <div className="page">
      <header className="topbar">
        <div>
          <div className="title">Dashboard</div>
          <div className="subtitle">IK1332 Elevator System</div>
        </div>
      </header>

      <main className="grid">
        <section className="card">
          <div className="cardTitle">Overview</div>
          <div className="cardBody"></div>
        </section>

        <section className="card">
          <div className="cardTitle">Activity</div>
          <div className="cardBody"></div>
        </section>

        <section className="card">
          <div className="cardTitle">Details</div>
          <div className="cardBody"></div>
        </section>

        <section className="card">
          <div className="cardTitle">Information</div>
          <div className="cardBody"></div>
        </section>
      </main>
    </div>
  );
}
