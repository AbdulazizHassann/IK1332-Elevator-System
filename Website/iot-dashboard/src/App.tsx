import { Routes, Route } from "react-router-dom";
import Dashboard from "./dashboard";
import Statistics from "./statistics";

export default function App() {
  return (
    <Routes>
      <Route path="/" element={<Dashboard />} />
      <Route path="/statistics/:sensor" element={<Statistics />} />
    </Routes>
  );
}
