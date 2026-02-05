const express = require("express");
const app = express();

app.use(express.json());

app.post("/api/elevator/data", (req, res) => {
  console.log(req.body);
  res.status(200).json({ status: "ok" });
});

app.get("/", (req, res) => {
  res.send("Elevator API running. what more is running.");
});

app.listen(4000, () => {
  console.log("Server running on port 4000");
});
