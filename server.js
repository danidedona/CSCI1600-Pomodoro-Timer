const express = require("express");
const app = express();

app.use(express.json());

let latest = { message: "No data yet" };

app.post("/update", (req, res) => {
  latest = req.body;
  console.log("Received:", latest);
  res.send("ok");
});

app.get("/latest", (req, res) => {
  res.json(latest);
});

app.use(express.static("."));

app.listen(3000, "0.0.0.0", () =>
  console.log("Server running on port 3000 (LAN accessible)")
);
