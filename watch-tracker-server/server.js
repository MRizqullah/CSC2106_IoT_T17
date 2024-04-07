const express = require("express");
const http = require("http");
const socketIo = require("socket.io");

const app = express();
const server = http.createServer(app);

const cors = require("cors");

// Allow CORS for all routes
app.use(cors());

// Configure Socket.IO to allow CORS
const io = socketIo(server, {
  cors: {
    origin: "http://localhost:3000", // URL of your React frontend
    methods: ["GET", "POST"],
  },
});

const watchNodes = {};
const nodeDevices = {};

app.use(express.json());

app.post("/api/data", (req, res) => {
  // Process incoming data from LoRa devices (similar to your Flask route)
  const { mac, rssi, node_mac } = req.body;
  // Update watchNodes and nodeDevices as in your Flask app
  // Emit updates to clients
  io.emit("tag_update", {
    mac_address: mac,
    node_type: "LoRa",
    rssi: rssi,
    node_mac: node_mac,
  });
  console.log("Emitted tag_update:", {
    mac_address: mac,
    node_type: "LoRa",
    rssi: rssi,
    node_mac: node_mac,
  });

  // Respond to the request
  res.json({ status: "success", message: "Data received" });
});

app.post("/api/ble_data", (req, res) => {
  // Process incoming data from BLE devices (similar to your Flask route)
  const data = req.body;
  // Update watchNodes and nodeDevices as in your Flask app
  // Emit updates to clients
  data.forEach((device) => {
    const { mac_address, rssi, boardName } = device;
    io.emit("tag_update", {
      mac_address: mac_address,
      node_type: "BLE",
      rssi: rssi,
      node_mac: boardName,
    });
    console.log("Emitted tag_update:", {
      mac_address: mac_address,
      node_type: "BLE",
      rssi: rssi,
      node_mac: boardName,
    });
  });
  // Respond to the request
  res.json({ status: "success", message: "BLE data received" });
});

io.on("connection", (socket) => {
  console.log("A client connected");
});

server.listen(5000, () => {
  console.log("Server listening on port 5000");
});
